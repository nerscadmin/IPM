
AC_INIT([ipmparse], [1.0.0], [ipm-dev@nersc.gov])
AC_CONFIG_SRCDIR([ipm_parse.cc])

AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE

AC_PROG_CXX

#if test "x$CC" != "xgcc"; then
#   echo "CC=gcc required... giving up"
#   exit 1
#fi 

AM_PROG_CC_C_O
AC_CONFIG_HEADERS(config.h)

AC_PROG_INSTALL

if test ! -f "cubew/lib/libcubew3.a"; then
	echo "build libcubew3 in cubew/lib first!"
	exit 1
else
	CXXFLAGS+=" -I./cubew/lib"
	LDFLAGS+=" -L./cubew/lib/"
	LIBS+=" -lcubew3"
fi

AC_CHECK_LIB(mxml, mxmlLoadFile, [], 
	[ echo "To compile ipm_parse you need to have libmxml installed." ]
)

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT


