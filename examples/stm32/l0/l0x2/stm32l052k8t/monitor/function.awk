# function.awk - Make xxx_func() from xxx.h.

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
    n = 0
    f = 0
    maxarg = 16
}
/^(void |int |bool |enum )[a-z][a-z0-9_ ]*\(/, /\);/ {
    if (f == 0) {
	type[n] = $1
	if ($1 == "enum")
	    s = 3
	else
	    s = 2
	fname[n] = $s
	sub(/\(.*/, "", fname[n])
	if ($s ~ /\);$/) {
	    args[n] = 0
	    n++
	} else {
	    t = $s
	    sub(/.*\(/, "", t)
	    a = 0
	    f = 1
	}
    }
    if (f == 1) {
	for (i = s + 1; i <= NF; i++) {
	    if ($i ~ /\);$/) {
		if ($i ~ /^\*/)
		    argtype[n * maxarg + a] = t " *"
		f = 0;
		break;
	    }
	    if ($i ~ /,$/) {
		if ($i ~ /^\*/)
		    argtype[n * maxarg + a] = t " *"
		a++
	    } else {
		t = $i
	    }
	}
	if (f == 0) {
	    args[n] = a + 1
	    n++
	} else {
	    s = 0
	}
    }
}
END {
    printf("/* This file was generated automatically - don't edit it. */\n\n")
    printf("#include <stdio.h>\n#include <stdbool.h>\n\n")
    printf("#include <%s.h>\n\n", dev)
    printf("#include \"monitor.h\"\n\n")
    printf("static const char *fname[] = {\n")
    for (i = 0; i < n; i++)
	printf("\t\"%s\",\n", fname[i])
    printf("\tNULL\n")
    printf("};\n\n")
    printf("int %s_func(int argc, char *argv[])\n", dev)
    printf("{\n")
    printf("\tint convert_arg(int argc, char *argv[], int a[]);\n")
    printf("\tint fname_cmp(char *s, const char *p[]);\n\n")
    printf("\tint a[MAXARG];\n\n")
    printf("\tif (convert_arg(argc, argv, a))\n")
    printf("\t\treturn -1;\n\n")
    printf("\tswitch (fname_cmp(argv[0], fname)) {\n")
    for (i = 0; i < n; i++) {
	printf("\tcase %d:\n", i)
	if (type[i] == "void")
	    printf("\t\t%s(", fname[i])
	else
	    printf("\t\tprintf(\"0x%%08x\\n\", %s(", fname[i])
	for (j = 0; j < args[i]; j++) {
	    k = i * maxarg + j
	    if (j)
		printf(", ")
	    if (k in argtype)
		printf("(%s)", argtype[k])
	    printf("a[%d]", j + 1)
	}
	if (type[i] == "void")
	    printf(");\n")
	else
	    printf("));\n")
	printf("\t\tbreak;\n")
    }
    printf("\tdefault:\n")
    printf("\t\tprintf(\"Unkown function: %%s\\n\", argv[0]);\n")
    printf("\t\treturn -1;\n")
    printf("\t}\n\n")
    printf("\treturn 0;\n")
    printf("}\n")
}
