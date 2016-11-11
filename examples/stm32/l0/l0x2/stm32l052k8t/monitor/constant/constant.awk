# constant.awk - Make xxx() from xxx.h.

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
    printf("/* This file was generated automatically - don't edit it. */\n\n")
    printf("#include <stdio.h>\n#include <string.h>\n#include <stdbool.h>\n\n")
    if (map != dev)
	printf("#include <%s.h>\n", map)
    printf("#include <%s.h>\n\n", dev)
    n = 0
    nl = 0
}
# /* --- XXXX registers       -- s = 0
# /* --- XXXX values          -- s = 1
# /* --- XXXX prototypes      -- s = 2
# /* --- STM32L0x2 memory map -- s = 3
# /* --- STM32L0 Cortex-M0+   -- s = 3
# /* --- Port definitions     -- s = 4
# /* --- Buffer descriptor    -- s = 5
# /* --- XXXX                 -- s = 8
/^\/\* --- [A-Za-z0-9]+ registers / {
    s = 0
    printf("void %s_register(void)\n{\n", dev)
    f[n] = "register"
    n++
    next
}
/^\/\* --- [A-Za-z0-9_/]+ values / {
    if (d == 1) {
	printf("}\n")
	d = 0
    }
    nl = 0
    s = 1
# register name: A/B -> A
    rname = tolower($3)
    sub(/[^A-Za-z0-9_].*/, "", rname)
    next
}
/^\/\* --- [A-Za-z]+ prototypes / {
    if (d == 1) {
	printf("}\n")
	d = 0
    }
    nl = 0
    s = 2
    next
}
/^\/\* --- (STM32L0x2 memory map|STM32L0 Cortex-M0\+) / {
    s = 3
    printf("void %s_memorymap(void)\n{\n", dev)
    f[n] = "memorymap"
    n++
    next
}
/^\/\* --- Port definitions / {
    if (d == 1) {
	printf("}\n")
	d = 0
    }
    nl = 0
    s = 4	
    if (n)
	printf("\n")
    printf("void %s_port(void)\n{\n", dev)
    f[n] = "port"
    n++
    next
}
/^\/\* --- Buffer descriptor / {
    if (d == 1) {
	printf("}\n")
	d = 0
    }
    nl = 0
    s = 5
    next
}
/^\/\* --- / && s != 3 {
    if (d == 1 || s == 4) {
	printf("}\n")
	d = 0
    }
    nl = 0
    s = 8
    next
}
/^\/\* / {
    if (d == 1 || e == 1)
	nl = 1
    if (s == 2) {
# enum comment: A-B -> AB
	ename = tolower($2)
	sub(/[^a-z0-9_]/, "", ename)
    }
    next
}
/^#define [A-Z][A-Z0-9_]*[ \t]+[A-Za-z0-9\(]+/ {
    if (s == 0 || s == 1 || s == 3 || s == 8) {
	if (d == 0) {
	    if (s == 1) {
		if (n)
		    printf("\n")
		printf("void %s_%s(void)\n{\n", dev, rname)
		f[n] = rname;
		n++
	    }

	    if (s == 8) {
		if (n)
		    printf("\n")
		printf("void %s_other(void)\n{\n", dev)
		f[n] = "other"
		n++
	    }

	    d = 1
	}

	if (nl == 1) {
	    printf("\tprintf(\"\\n\");\n")
	    nl = 0
	}

	if (s == 0)
	    printf("\tprintf(\"%-40s0x%%08x\\n\", (unsigned int)%s);\n", \
		   $2, $2)
	if (s == 1 || s == 3)
	    printf("\tprintf(\"%-40s0x%%08x\\n\", %s);\n", $2, $2)
	if (s == 8)
	    printf("\tprintf(\"%-40s0x%%08x %%d\\n\", %s, %s);\n", $2, $2, $2)
    }
    next
}
/^enum( [a-z][a-z0-9_]*)? {/ {
    if (s == 2) {
	if ($2 != "{")
	    ename = $2
	if (n)
	    printf("\n")
	printf("void %s_%s(void)\n{\n", dev, ename)
	f[n] = ename
	n++
    }
    e = 1
    if (s == 2)
	fl = 1
    if (s == 4) {
	if (fl == 0)
	    fl = 1
	else
	    fl = 0
	nl = 1
    }
    next
}
/^};/ {
    e = 0
    if (s == 2)
	printf("}\n")
    next
}
e == 1 {
    if (NF < 1) {
	nl = 1
    } else {
	if (fl == 0 && nl == 1)
	    printf("\tprintf(\"\\n\");\n")
	nl = 0
	if (s != 4)
	    fl = 0
	if ($2 == "=") {
	    printf("\tprintf(\"%-40s0x%%08x\\n\", %s);\n", $1, $1)
	} else {
	    for (i = 1; i <= NF; i++) {
		if ($i == "/*")
		    break;
		t = $i
		sub(/,/, "", t)
		printf("\tprintf(\"%-40s0x%%08x %%d\\n\", %s, %s);\n", t, t, t)
	    }
	}
    }
}
END {
    if (s == 3)
	printf("}\n")
    if (n == 1) {
	printf("\nvoid %s(char *category)\n{\n", dev)
	printf("\t%s_%s();\n", dev, f[0])
	printf("}\n")
    } else {
	printf("\nvoid %s_help(void)\n{\n", dev)
	printf("\tprintf(\"category2:\\n\");\n")
	for (i = 0; i < n; i++)
	    printf("\tprintf(\"\t%s\\n\");\n", f[i])
	printf("}\n")
	printf("\nvoid %s(char *category)\n{\n", dev)
	for (i = 0; i < n; i++) {
	    printf("\t")
	    if (i != 0)
		printf("else ")
	    printf("if (strcmp(category, \"%s\") == 0)\n", f[i])
	    printf("\t\t%s_%s();\n", dev, f[i])
	}
	printf("\telse\n\t\t%s_help();\n}\n", dev)
    }
}
