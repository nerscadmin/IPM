
AC_INIT([ipm], [2.0.2], [ipm-dev@nersc.gov])
AC_CONFIG_SRCDIR([src/ipm_core.c])

AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([no-dependencies])
dnl AC_CONFIG_MACRO_DIR([m4])

sinclude(m4/ax_mpi.m4)
sinclude(m4/ipm_underscore.m4)
sinclude(m4/ipm_mpistatuscount.m4)
sinclude(m4/ipm_hostdetection.m4)

dnl CRAY is not the default
CRAY=0    
AX_IPM_HOSTDETECTION


dnl AC_PROG_CC
AC_PROG_CXX
dnl AC_PROG_F77
dnl AC_PROG_FC


if test "x$MPICC" == "x"; then
AC_LANG([C])
AX_MPI
AC_LANG([Fortran 77])
AX_MPI
AC_LANG([Fortran])
AX_MPI
AC_LANG([C])
fi

AM_PROG_CC_C_O
AC_CONFIG_HEADERS(config.h)

AX_IPM_UNDERSCORE
AX_IPM_MPISTATUSCOUNT

if test "x$CRAY" = "x0"; then
   CC=$MPICC
   F77=$MPIF77
   AC_CHECK_HEADERS([dlfcn.h], 
	[
          CFLAGS="$CFLAGS -DHAVE_DYNLOAD"
	], 
	[], [])
   need_c2f_macros=no
   AC_CHECK_FUNC(MPI_Status_c2f, [], [need_c2f_macros=yes])
   AC_CHECK_FUNC(MPI_Status_f2c, [], [need_c2f_macros=yes])

   if test "x$need_c2f_macros" == "xyes"; then
 	CFLAGS="$CFLAGS -DNEED_C2F_MACROS"
   fi
else
   AC_DEFINE(HAVE_DLFCN_H, 0)
fi

AC_PROG_INSTALL
AC_PROG_LIBTOOL


# defining the enabled modules
enable_mod_mpi=yes
enable_mod_posixio=no
enable_mod_mpiio=no
enable_mod_self_monitor=no
enable_mod_mem=no
enable_mod_callpath=no
enable_mod_procctrl=no
enable_mod_keyhist=no
enable_mod_omptracepoints=no 
enable_mod_clustering=no
enable_mod_cuda=no
enable_mod_cublas=no
enable_mod_cufft=no


AC_ARG_ENABLE(mpi,
  AC_HELP_STRING([--disable-mpi], [disable mpi module]),
  [
	AS_IF([test "x$enableval" = "xno"], 
      [enable_mod_mpi=no])  
  ],
  [ enable_mod_mpi=yes  ]
)
    
AC_ARG_ENABLE(posixio,
  AC_HELP_STRING([--enable-posixio], [enabled posixio module (default=no)]),
  [
     AS_IF([test "x$enableval" = "xyes"],
     	[enable_mod_posixio=yes])  
  ],
  [ enable_mod_posixio=no ]  
)
 
 
have_cudainc=no 
AC_ARG_WITH(cudapath,
    [ --with-cudapath=<path> to cuda installation.],
    [
    if test -d "$withval"; then
            CFLAGS="$CFLAGS -I$withval/include"
            have_cudainc=yes
        else
            if test "x$withval" == "xyes"; then
                echo "cuda path is empty."
            else
                echo "cuda was not found in $withval."
            fi
            exit 1
    fi
    report_experimental=yes
    ],
    [
        have_cudainc=no
    ]
) 
 


report_experimental=no 
AC_ARG_ENABLE(experimental,
  AC_HELP_STRING(
  	[--enable-experimental=MODULE-LIST], 
  	[commas-separated list of IPMv2 modules to enable]),
  [
  	report_experimental=yes
  	enableval=`echo $enableval | sed s/\,/\ /g`
    for i in $enableval; do
      if test "$i" = "papi"; then
      	echo "to enable papi use --with-papi=<path>"
      	exit 1 	
      elif test "$i" = "all"; then
#      	enable_mod_mpi=yes
		enable_mod_posixio=yes
	  	enable_mod_mpiio=yes
      	enable_mod_self_monitor=yes
#      	enable_mod_mem=yes
#      	enable_mod_callpath=yes
#      	enable_mod_procctrl=yes
#      	enable_mod_keyhist=yes
      	enable_mod_omptracepoints=yes 
#      	enable_mod_clustering=yes
      	enable_mod_cuda=yes
      	enable_mod_cublas=yes
      	enable_mod_cufft=yes
      else
dnl        i=`echo $i | sed 's/-/_/g'`
        eval "enable_mod_$i=yes"
      fi
      
      if test "$enable_mod_cuda" = "yes" -o "$enable_mod_cublas" = "yes" -o "$enable_mod_cufft" = "yes" ; 
      then
        if test "$have_cudainc" = "no";
        then
            echo "you need to set the path to cuda via --with-cudapath"
            exit 1
        fi
      fi
            
    done
  ])
     
  
AC_ARG_ENABLE(coll-details,
  AC_HELP_STRING(
  [--enable-coll-details], 
  [Enable detailed monitoring of collective operations (experimental)]),
  [CFLAGS="$CFLAGS -DIPM_COLLECTIVE_DETAILS=1"],
  [])

AC_ARG_WITH(papi,
    [ --with-papi=<path> to PAPI installation.],
    [
   	if test -d "$withval"; then
	   		CFLAGS="$CFLAGS -I$withval/include"
			LDFLAGS="$LDFLAGS -L$withval/lib -Wl,-rpath=$withval/lib"
			LIBS="$LIBS -lpapi"
			echo "papi: $withval"
			enable_mod_papi=yes
		else
			if test "x$withval" == "xyes"; then
				echo "no path to papi given."
			else
				echo "papi not found in $withval."
			fi
			exit 1
    fi
    ],
    [
			enable_mod_papi=no
    ]
)


report_libunwind=no
AC_ARG_WITH(libunwind,
    [ --with-libunwind=<path> to libunwind installation.],
    [
    report_libunwind=yes
   	if test -d "$withval"; then
	   		CFLAGS="$CFLAGS -I$withval/include"
			LDFLAGS="$LDFLAGS -L$withval/lib"
			LIBS="$LIBS -lunwind"
			echo "libunwind: $withval"
			have_libunwind=yes
		else
			if test "x$withval" == "xyes"; then
				echo "no path to libunwind given."
			else
				echo "libunwind not found in $withval."
			fi
			exit 1
    fi
    ],
    [
			have_libunwind=no
    ]
)  

AM_CONDITIONAL([ENABLE_MPI], [test "$enable_mod_mpi" = yes])
AM_CONDITIONAL([ENABLE_SELF_MONITORING], [test "$enable_mod_self_monitor" = yes])
AM_CONDITIONAL([ENABLE_MPIIO], [test "$enable_mod_mpiio" = yes])
AM_CONDITIONAL([ENABLE_MEM], [test "$enable_mod_mem" = yes])
AM_CONDITIONAL([ENABLE_POSIXIO], [test "$enable_mod_posixio" = yes])
AM_CONDITIONAL([ENABLE_CALLPATH], [test "$enable_mod_callpath" = yes])
AM_CONDITIONAL([ENABLE_PAPI], [test "$enable_mod_papi" = yes])
AM_CONDITIONAL([ENABLE_KEYHIST], [test "$enable_mod_keyhist" = yes])
AM_CONDITIONAL([ENABLE_PROCCTRL], [test "$enable_mod_procctrl" = yes])
AM_CONDITIONAL([ENABLE_OMPTRACEPOINTS], [test "$enable_mod_omptracepoints" = yes])
AM_CONDITIONAL([ENABLE_CLUSTERING], [test "$enable_mod_clustering" = yes])
AM_CONDITIONAL([ENABLE_CUDA], [test "$enable_mod_cuda" = yes])
AM_CONDITIONAL([ENABLE_CUBLAS], [test "$enable_mod_cublas" = yes])
AM_CONDITIONAL([ENABLE_CUFFT], [test "$enable_mod_cufft" = yes])

if test "x$target_os" == "xlinux-gnu"; then
	CFLAGS+=" -DOS_LINUX"
fi


enable_ipm_parser=no
AC_ARG_ENABLE(parser, 
  AC_HELP_STRING(
    [--enable-parser], 
    [Enables building the ipm parser.])
, 
[
AS_IF([test "x$enable_parser" = "xyes"], [
enable_ipm_parser=yes

AC_CHECK_LIB(mxml, mxmlElementGetAttr, [], 
 [ 
	echo "To compile ipm_parse you need to have libmxml installed."
	exit 1
 ])

if test ! -f "utils/cubew/lib/libcubew3.a"; then
        echo "trying to build libcubew3 in utils/cubew/lib for you"
        make -C utils/cubew/lib
fi

if test ! -f "utils/cubew/lib/libcubew3.a"; then
        echo "bilding libcubew3 in utils/cubew/lib failed!"
        exit 1
fi
]
)
]
, 
[:] dnl not given
)

AM_CONDITIONAL([ENABLE_PARSER], [test "$enable_ipm_parser" = yes])


AC_CONFIG_FILES([
Makefile
bin/Makefile
etc/Makefile
src/Makefile
utils/Makefile
test/Makefile
test/test.allgather/Makefile
test/test.allgatherv/Makefile
test/test.allreduce/Makefile
test/test.alltoall/Makefile
test/test.alltoallv/Makefile
test/test.bcast/Makefile
test/test.fhello/Makefile
test/test.forever/Makefile
test/test.fork/Makefile
test/test.fring/Makefile
test/test.gather/Makefile
test/test.gatherv/Makefile
test/test.hello/Makefile
test/test.jacobi/Makefile
test/test.keyhist/Makefile
test/test.omp/Makefile 
test/test.pcontrol/Makefile 
test/test.posixio-helloworld/Makefile
test/test.posixio-read/Makefile
test/test.simple_mpi/Makefile
test/test.status_ignore/Makefile 
])

dnl test/test.pomp-standalone/Makefile

AC_OUTPUT


echo "################################################"
echo " IPM configuration:"
echo "     MPI profiling enabled       : $enable_mod_mpi"
echo "     POSIX-I/O profiling enabled : $enable_mod_posixio"
echo "     PAPI enabled                : $enable_mod_papi"
if test "x$report_experimental" == "xyes"; then
echo "     Self monitoring enabled     : $enable_mod_self_monitor"
echo "     MPIIO (C only) enabled      : $enable_mod_mpiio"
#echo "     MEM   enabled               : $enable_mod_mem"
#echo "     CALLPATH enabled            : $enable_mod_callpath"
#echo "     keyhist enabled             : $enable_mod_keyhist"
#echo "     procctrl enabled            : $enable_mod_procctrl"
echo "     OpenMP (PGI/Cray) enabled   : $enable_mod_omptracepoints"
#echo "     clustering enabled          : $enable_mod_clustering"
echo "     CUDA profiling enabled      : $enable_mod_cuda"
echo "     CUBLAS profiling enabled    : $enable_mod_cublas"
echo "     CUFFT profiling enabled     : $enable_mod_cufft"
fi
if test "x$report_libunwind" == "xyes"; then
echo "     use libunwind               : $have_libunwind"
fi
echo "     CFLAGS                      : $CFLAGS"
echo "     LDFLAGS                     : $LDFLAGS"
echo "     LIBS                        : $LIBS"
echo "     MPI_STATUS_COUNT            : $IPM_MPISTATUSCOUNT"
echo "     Fortran underscore          : $IPM_FUNDERSCORE"
echo "     Building IPM Parser         : $enable_ipm_parser"
echo ""


warn=yes
check="$enable_mod_mpi $enable_mod_posixio $enable_mod_papi $enable_mod_cuda $enable_mod_cufft $enable_mod_cublas $report_experimental"
for mod in $check
do
  if test "x$mod" = "xyes"; then
    warn=no
  fi
done 

if test "x$warn" = "xyes"; then
echo ""
echo "################################################"
echo " !! WARNING !!" 
echo " No module of IPM has been enabled!"
echo "################################################"
fi
