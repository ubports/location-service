#!/bin/sh

# Copyright (C) 2013 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Michi Henning <michi.henning@canonical.com>

# Simple script to run the code base through astyle, followed by clang-format (which
# undoes some damage that's done by astyle, without wiping out astyle edits we want
# to happen).
#
# If either program makes a mess of some file such that it won't compile anymore
# or otherwise gets adorned with unacceptable edits, add the file to the list
# of files to filter out (grep -v below).

usage()
{
    echo usage: format-files project_dir astyle_cmd clang_format_cmd 2>&1
    exit 1
}

[ $# -ne 3 ] && usage

dir="$1"
astyle="$2"
format="$3"

files=`find "$dir" -name '*.h' -o -name '*.cpp' -o -name '*.c' \
    | grep -v UnityScopesApi_tp.h`

"$astyle" --options="$dir"/astyle-config -n $files
[ $? -ne 0 ] && exit $?

# astyle 2.03 writes DOS line endings: https://sourceforge.net/p/astyle/bugs/268/
dos2unix -q $files

"$format" -i -style=file $files
exit $?
