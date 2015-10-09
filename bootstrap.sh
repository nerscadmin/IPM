#!/bin/bash

set -e

if [ -f Makefile ]; then
	make distclean
fi

autoreconf --install --force

if ! grep -q "Expand the sysroot to ease extracting the directories later" m4/libtool.m4; then
	echo "error: could not find target key for patching libtool.m4 - please report this to the IPM developers"
	exit 1
fi

mv m4/libtool.m4 m4/libtool.m4_orig

awk '/Expand the sysroot to ease extracting the directories later/ { printf("\
       # Some compilers *also* place space between \"-l\" and the library name.\n\
       # Remove the space.\n\
       if test $p = \"-l\"; then\n\
	 prev=$p\n\
	 continue\n\
       fi\n\n\
"); } { print $0 }' m4/libtool.m4_orig > m4/libtool.m4
