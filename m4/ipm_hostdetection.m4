

AC_DEFUN([AX_IPM_HOSTDETECTION], [
AC_MSG_CHECKING([detecting IPM host])

IPM_HOST="x"

HOSTNAME=$(hostname)

case "$HOSTNAME" in
 nid*) IPM_HOST=franklin ;;
 jacin*) IPM_HOST=jacquard ;;
 lslogin*) IPM_HOST=lonestar ;;
 *ranger.tacc.utexas.edu) IPM_HOST=ranger;;
 *.bigben.psc.teragrid.org) IPM_HOST=bigben ;;
 *.pople.psc.teragrid.org) IPM_HOST=pople;;
 jaguar*) IPM_HOST=jaguar;;
 kraken*) IPM_HOST=kraken;;
 1) ;;
 *) IPM_HOST=unknown;;
esac


  env=`echo $PE_ENV`
  case $env in
	PGI|PATHSCALE|CRAY|GNU)
		AC_DISABLE_SHARED
		MPICC=cc
		MPIF77=ftn
		CFLAGS="$CFLAGS -DIPM_DISABLE_PWENT -DLINUX_XT5"
		CRAY=1
      ;;
	*) 
		AC_ENABLE_SHARED
		CRAY=0
      ;;
  esac
  
  case $env in
      PGI)
        CC=pgcc
        CXX=pgcpp
        F77=pgf77
      ;;
      PATHSCALE)
        CC=pathcc
        CXX=pathCC
        F77=pathf95
      ;;
      GNU)
        CC=gcc
        CXX=g++
        F77=gfortran
      ;;
      CRAY)
        CC=cc
        CXX=CC
        F77=ftn
      ;;
      *)
      ;;
  esac


AC_MSG_RESULT( $IPM_HOST )
])
