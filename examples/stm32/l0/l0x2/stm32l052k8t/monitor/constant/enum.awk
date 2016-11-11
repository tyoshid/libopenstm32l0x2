# enum.awk - Make enumconst[] from header files.

# Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

BEGIN {
    cn = 0
    fn = 0
}
/^enum( [a-z][a-z0-9_]*)? {/, /^};/ {
    if (NF >= 1 && $1 != "enum" && $1 != "};" && $1 != "/*") {
	if ($2 == "=") {
	    constant[cn] = $1
	    cn++
	} else {
	    for (i = 1; i <= NF; i++) {
		if ($i == "/*")
		    break
		constant[cn] = $i
		sub(/,/, "", constant[cn])
		cn++
	    }
	}
	if (fn in fname) {
	    if (fname[fn] != FILENAME) {
		fn++
		fname[fn] = FILENAME
	    }
	} else {
	    fname[fn] = FILENAME
	}
    }
}
END {
    printf("/* This file was generated automatically - don't edit it. */\n\n")
    printf("#include <stdbool.h>\n\n")
    for (i = 0; i <= fn; i++) {
	sub(/.*\//, "", fname[i])
	printf("#include <%s>\n", fname[i])
    }
    printf("\n#include \"symtab.h\"\n")
    printf("\nstruct symtab enumconst[] = {\n")
    for (i = 0; i < cn; i++)
	printf("\t{\"%s\", %s},\n", constant[i], constant[i])
    printf("\t{0, 0}\n")
    printf("};\n")
}
