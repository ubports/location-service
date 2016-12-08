// Copyright (C) 2016 Thomas Voss <thomas.voss.bochum@gmail.com>
//
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <location/providers/ubx/_8/nmea/generator.h>
#include <location/providers/ubx/_8/nmea/grammar.h>

#include <iterator>

namespace nmea = location::providers::ubx::_8::nmea;

namespace {
template <typename Iterator>
std::uint8_t checksum(Iterator it, Iterator itE) {
  std::uint8_t result = 0;
  for (; it != itE; ++it) result ^= *it;

  return result;
}
}

nmea::Sentence nmea::parse_sentence(const std::string& message) {
  using boost::phoenix::ref;
  using boost::spirit::qi::hex;
  using boost::spirit::qi::lit;
  using boost::spirit::qi::ascii::char_;
  using boost::spirit::qi::_1;
  using boost::spirit::qi::as_string;

  std::string s;
  std::uint32_t cs;

  auto set_s = [&s](const std::string& in) { s = in; };

  auto set_cs = [&cs](std::uint32_t in) { cs = in; };

  if (not boost::spirit::qi::parse(message.begin(), message.end(),
                                   (lit('$') >> as_string[*(~char_('*'))][set_s] >> lit('*') >> hex[set_cs] >> "\r\n")))
    throw std::runtime_error("Failed to unmarshal NMEA message: " + message);

  nmea::Sentence sentence;
  if (not boost::spirit::qi::parse(s.begin(), s.end(), nmea::Grammar<std::string::iterator>(), sentence))
    throw std::runtime_error("Failed to parse NMEA sentence: " + s);

  if (checksum(s.begin(), s.end()) != cs) throw std::runtime_error("Failed to verify NMEA message integrity.");

  return sentence;
}

std::string nmea::generate_sentence(const Sentence& sentence) {
  using boost::spirit::karma::hex;
  using boost::spirit::karma::lit;
  using boost::spirit::karma::upper;

  std::string s;
  std::back_insert_iterator<std::string> its(s);
  boost::spirit::karma::generate(its, Generator<std::back_insert_iterator<std::string>>(), sentence);

  std::string result;
  std::back_insert_iterator<std::string> itr(result);
  if (not boost::spirit::karma::generate(itr, lit('$') << s << lit('*') << upper[hex] << "\r\n",
                                         checksum(s.begin(), s.end())))
    throw std::logic_error("Failed to marshal NMEA message");

  return result;
}

std::ostream& nmea::operator<<(std::ostream& out, const Sentence& sentence) {
  return out << generate_sentence(sentence);
}
