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

#ifndef UBX_8_WRITER_H_
#define UBX_8_WRITER_H_

#include <cstdint>
#include <vector>

namespace location
{
namespace providers
{
namespace ubx
{
namespace _8
{

class Writer
{
public:
    explicit Writer(std::vector<std::uint8_t>::iterator begin,
                    std::vector<std::uint8_t>::iterator end);

    void write_unsigned_char(std::uint8_t value);
    void write_signed_char(std::int8_t value);
    void write_unsigned_short(std::uint16_t value);
    void write_signed_short(std::int16_t value);
    void write_unsigned_long(std::uint32_t value);
    void write_signed_long(std::int32_t value);
    void write_float(float value);
    void write_double(double value);
    void write_string(const char* s, std::size_t size);

    Writer slice(std::size_t size);

private:
    std::vector<std::uint8_t>::iterator begin;
    std::vector<std::uint8_t>::iterator current;
    std::vector<std::uint8_t>::iterator end;
};

}  // namespace _8
}  // namespace ubx
}  // namespace providers
}  // namespace location

#endif // UBX_8_WRITER_H_
