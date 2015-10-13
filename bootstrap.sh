#!/bin/bash

set -e

usage () {
	echo "Usage: $0 [--verbose] [--help]"
	echo "  --verbose : display autoreconf warnings"
	echo "  --help : display this message"
}

# parse args
warnings="--warnings=none"
if [ $# -eq 1 ]; then
	if [ "$1" == "--verbose" ]; then
		warnings=""
	elif [ "$1" == "--help" ]; then
		usage
		exit 0
	else
		usage
		exit 1
	fi
elif [ $# -ne 0 ]; then
	usage
	exit 1
fi

# clean up earlier configuration
if [ -f Makefile ]; then
	make distclean
fi

# initial sweep - install missing files, overwriting previous state
autoreconf --install --force $warnings

# check whether we can find the insertion point for our libtool.m4 bugfix
if ! grep -q "\-L\* | \-R\* | \-l\*)" m4/libtool.m4; then
	echo "error: could not find target key for patching libtool.m4 - please report this to the IPM developers along with your m4/libtool.m4"
	exit 1
fi

# apply the fix
mv m4/libtool.m4 m4/libtool.m4_orig
awk '{ print $0 } /-L\* \| -R\* \| -l\*\)/ { printf("\n\t# Some compilers *also* place space between \"-l\" and the library name.\n\t# Remove the space.\n\tif test $p = \"-l\"; then prev=$p; continue; fi\n\n"); }' m4/libtool.m4_orig > m4/libtool.m4

# autoreconf once more (no --force) to assimilate libtool.m4 changes
autoreconf $warnings
