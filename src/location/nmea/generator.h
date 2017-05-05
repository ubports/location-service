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
#ifndef LOCATION_NMEA_GENERATOR_H_
#define LOCATION_NMEA_GENERATOR_H_

#define FUSION_MAX_VECTOR_SIZE 15

#include <location/nmea/sentence.h>

#include <location/nmea/fusion_adapt.h>

#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_real.hpp>
#include <boost/spirit/include/karma_uint.hpp>

#include <cstdint>

namespace location
{
namespace nmea
{

template <unsigned prec, typename T>
struct FinitePrecisionPolicy : boost::spirit::karma::real_policies<T>
{
    static int floatfield(T) { return boost::spirit::karma::real_policies<T>::fmtflags::fixed; }
    static unsigned precision(T) { return prec; }
};

template <typename Iterator>
class Generator : public boost::spirit::karma::grammar<Iterator, Sentence()>
{
public:
    Generator() : Generator::base_type{start}
    {
        // clang-format off
    start %= (gga | gsa | gll | gsv | rmc | txt | vtg);
    field_separator %= ',';
    checksum %= boost::spirit::karma::hex;
    talker.add
        (Talker::ga, "GA")
        (Talker::gl, "GL")
        (Talker::gn, "GN")
        (Talker::gp, "GP");
    status.add
        (Status::valid, 'A')
        (Status::not_valid, 'V');
    mode.add
        (Mode::autonomous, 'A')
        (Mode::differential, 'D')
        (Mode::estimated, 'E')
        (Mode::manual_input, 'M')
        (Mode::simulator_mode, 'S')
        (Mode::data_not_valid, 'N');
    cardinal_direction.all.add
        (CardinalDirection::north, 'N')
        (CardinalDirection::south, 'S')
        (CardinalDirection::east, 'E')
        (CardinalDirection::west, 'W');
    cardinal_direction.ns.add
        (CardinalDirection::north, 'N')
        (CardinalDirection::south, 'S');
    cardinal_direction.ew.add
        (CardinalDirection::east, 'E')
        (CardinalDirection::west, 'W');
    gps.fix_mode.add
        (gps::FixMode::invalid, '0')
        (gps::FixMode::gps_sps, '1')
        (gps::FixMode::differential_gps_sps, '2')
        (gps::FixMode::gps_pps, '3')
        (gps::FixMode::real_time_kinematic, '4')
        (gps::FixMode::floating_point_real_time_kinematic, '5')
        (gps::FixMode::estimated, '6')
        (gps::FixMode::manual_input, '7')
        (gps::FixMode::simulator, '8');
    for_gsa.operation_mode.add
        (Gsa::OperationMode::automatic, 'A')
        (Gsa::OperationMode::manual, 'M');    
    for_gsa.fix_mode.add
        (Gsa::FixMode::fix_not_available, '1')
        (Gsa::FixMode::fix_in_2d, '2')
        (Gsa::FixMode::fix_in_3d, '3');

    pdop %= boost::spirit::karma::real_generator<float, FinitePrecisionPolicy<5, float>>();
    hdop %= boost::spirit::karma::real_generator<float, FinitePrecisionPolicy<5, float>>();
    vdop %= boost::spirit::karma::real_generator<float, FinitePrecisionPolicy<5, float>>();

    latitude %= boost::spirit::karma::right_align(2, '0')[boost::spirit::karma::uint_generator<std::uint32_t, 10>()] <<
                boost::spirit::karma::real_generator<double, FinitePrecisionPolicy<5, double>>();

    longitude %= boost::spirit::karma::right_align(3, '0')[boost::spirit::karma::uint_generator<std::uint32_t, 10>()] <<
                 boost::spirit::karma::real_generator<double, FinitePrecisionPolicy<5, double>>();

    date %= boost::spirit::karma::right_align(2, '0')[boost::spirit::karma::uint_generator<std::uint32_t, 10>()] <<
            boost::spirit::karma::right_align(2, '0')[boost::spirit::karma::uint_generator<std::uint32_t, 10>()] <<
            boost::spirit::karma::right_align(2, '0')[boost::spirit::karma::uint_generator<std::uint32_t, 10>()];

    utc %= boost::spirit::karma::right_align(2, '0')[boost::spirit::karma::uint_generator<std::uint32_t, 10>()] <<
           boost::spirit::karma::right_align(2, '0')[boost::spirit::karma::uint_generator<std::uint32_t, 10>()] <<
           boost::spirit::karma::real_generator<double, FinitePrecisionPolicy<5, double>>();

    gsa %= talker << "GSA"                                                                          << field_separator
                  << -for_gsa.operation_mode                                                        << field_separator
                  << -for_gsa.fix_mode                                                              << field_separator
                  << boost::spirit::karma::repeat(12)[boost::spirit::karma::right_align(2, '0')
                     [-boost::spirit::karma::uint_generator<std::uint32_t, 10>()]                   << field_separator]
                  << -pdop                                                                          << field_separator
                  << -hdop                                                                          << field_separator
                  << -vdop;

    gga %= talker << "GGA"                                                                                                  << field_separator
                  << -utc                                                                                                   << field_separator
                  << -(latitude)                                                                                            << field_separator
                  << -(cardinal_direction.ns)                                                                               << field_separator
                  << -(longitude)                                                                                           << field_separator
                  << -(cardinal_direction.ew)                                                                               << field_separator
                  << -gps.fix_mode                                                                                          << field_separator
                  << boost::spirit::karma::right_align(2, '0')[-boost::spirit::karma::uint_generator<std::uint32_t, 10>()]  << field_separator
                  << -hdop                                                                                                  << field_separator
                  << -(boost::spirit::karma::real_generator<float>())                                                       << field_separator
                  << -boost::spirit::karma::lit('M')                                                                        << field_separator
                  << -(boost::spirit::karma::real_generator<float>())                                                       << field_separator
                  << -boost::spirit::karma::lit('M')                                                                        << field_separator
                  << -boost::spirit::karma::real_generator<float>()                                                         << field_separator
                  << boost::spirit::karma::right_align(2, '0')[-boost::spirit::karma::uint_generator<std::uint32_t, 10>()];

    gll %= talker << "GLL"                  << field_separator
                  << -latitude              << field_separator
                  << -cardinal_direction.ns << field_separator
                  << -longitude             << field_separator
                  << -cardinal_direction.ew << field_separator
                  << -utc                   << field_separator
                  << -status                << field_separator
                  << -mode;

    gsv_info %= field_separator <<
                boost::spirit::karma::right_align(2, '0')[-boost::spirit::karma::uint_generator<std::uint8_t, 10>()] <<
                field_separator <<
                boost::spirit::karma::right_align(2, '0')[-boost::spirit::karma::uint_generator<std::uint8_t, 10>()] <<
                field_separator <<
                boost::spirit::karma::right_align(3, '0')[-boost::spirit::karma::uint_generator<std::uint16_t, 10>()] <<
                field_separator <<
                boost::spirit::karma::right_align(2, '0')[-boost::spirit::karma::uint_generator<std::uint8_t, 10>()];

    gsv %= talker << "GSV" << field_separator
                  << boost::spirit::karma::maxwidth(1)[-boost::spirit::karma::uint_generator<std::uint8_t, 10>()]           << field_separator
                  << boost::spirit::karma::maxwidth(1)[-boost::spirit::karma::uint_generator<std::uint8_t, 10>()]           << field_separator
                  << boost::spirit::karma::right_align(2, '0')[-boost::spirit::karma::uint_generator<std::uint16_t, 10>()]
                  << *(gsv_info);

    rmc %= talker << "RMC"                                                                              << field_separator
                  << -utc                                                                               << field_separator
                  << -status                                                                            << field_separator
                  << -(latitude)                                                                        << field_separator
                  << -(cardinal_direction.ns)                                                           << field_separator
                  << -(longitude)                                                                       << field_separator
                  << -(cardinal_direction.ew)                                                           << field_separator
                  << -(boost::spirit::karma::real_generator<float, FinitePrecisionPolicy<5, float>>())  << field_separator
                  << -(boost::spirit::karma::real_generator<float, FinitePrecisionPolicy<5, float>>())  << field_separator
                  << -date                                                                              << field_separator
                  << -(boost::spirit::karma::real_generator<float, FinitePrecisionPolicy<5, float>>())  << field_separator
                  << -(cardinal_direction.ew)                                                           << field_separator
                  << -mode;

    txt %= talker << "TXT" << field_separator
                  << boost::spirit::karma::right_align(2, '0')[-boost::spirit::karma::uint_generator<std::uint8_t, 10>()] << field_separator
                  << boost::spirit::karma::right_align(2, '0')[-boost::spirit::karma::uint_generator<std::uint8_t, 10>()] << field_separator
                  << boost::spirit::karma::right_align(2, '0')[-boost::spirit::karma::uint_generator<std::uint8_t, 10>()] << field_separator
                  << -boost::spirit::karma::ascii::string;

    vtg %= talker << "VTG"                                                                            << field_separator
                  << -boost::spirit::karma::real_generator<float, FinitePrecisionPolicy<5, float>>()  << field_separator
                  << 'T'                                                                              << field_separator
                  << -boost::spirit::karma::real_generator<float, FinitePrecisionPolicy<5, float>>()  << field_separator
                  << 'M'                                                                              << field_separator
                  << -boost::spirit::karma::real_generator<float, FinitePrecisionPolicy<5, float>>()  << field_separator
                  << 'N'                                                                              << field_separator
                  << -boost::spirit::karma::real_generator<float, FinitePrecisionPolicy<5, float>>()  << field_separator
                  << 'K'                                                                              << field_separator
                  << -mode;
        // clang-format on
    }

private:
    boost::spirit::karma::rule<Iterator, Sentence()> start;
    boost::spirit::karma::rule<Iterator> field_separator;
    boost::spirit::karma::rule<Iterator, std::uint32_t()> checksum;
    boost::spirit::karma::rule<Iterator, Latitude()> latitude;
    boost::spirit::karma::rule<Iterator, Longitude()> longitude;
    boost::spirit::karma::rule<Iterator, Date()> date;
    boost::spirit::karma::rule<Iterator, Utc()> utc;
    boost::spirit::karma::rule<Iterator, float()> pdop;
    boost::spirit::karma::rule<Iterator, float()> hdop;
    boost::spirit::karma::rule<Iterator, float()> vdop;

    boost::spirit::karma::rule<Iterator, Gsa()> gsa;
    boost::spirit::karma::rule<Iterator, Gga()> gga;
    boost::spirit::karma::rule<Iterator, Gll()> gll;
    boost::spirit::karma::rule<Iterator, Gsv::Info()> gsv_info;
    boost::spirit::karma::rule<Iterator, Gsv()> gsv;
    boost::spirit::karma::rule<Iterator, Rmc()> rmc;
    boost::spirit::karma::rule<Iterator, Txt()> txt;
    boost::spirit::karma::rule<Iterator, Vtg()> vtg;

    // Enumerations go here
    boost::spirit::karma::symbols<Talker, const char*> talker;
    boost::spirit::karma::symbols<Status, char> status;
    boost::spirit::karma::symbols<Mode, char> mode;
    struct
    {
        boost::spirit::karma::symbols<CardinalDirection, char> all;
        boost::spirit::karma::symbols<CardinalDirection, char> ns;
        boost::spirit::karma::symbols<CardinalDirection, char> ew;
    } cardinal_direction;

    // Positioning-system specific rules go here
    struct
    {
        boost::spirit::karma::symbols<gps::FixMode, char> fix_mode;
    } gps;

    // Sentence-specific rules go here
    struct
    {
        boost::spirit::karma::symbols<Gsa::OperationMode, char> operation_mode;
        boost::spirit::karma::symbols<Gsa::FixMode, char> fix_mode;
    } for_gsa;
};

}
}

#endif // LOCATION_NMEA_GENERATOR_H_
