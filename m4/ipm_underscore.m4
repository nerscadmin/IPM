
###############################################################################
#
# tests for wrapper convention for Fortran
# requires ACX_MPI
# outputs autoconf variable IPM_FUNDERSCORE
#
# Copyright (c) 2009 Sascha Hunold <sascha@icsi.berkeley.edu>
# 
###############################################################################

AC_DEFUN([AX_IPM_UNDERSCORE], [
AC_MSG_CHECKING([underscores for F77 objects])

CWD=$PWD
TEST_DIR="$PWD/.test"
CONFIG_LOG=config.test.log

#echo "MPIF77 = ${MPIF77}"

## determine underscoring in a MPI F77 program {
if test "x$MPIF77" != "x" ; then 
TEST_NAME="simplef_mpi_underscores"

#echo -n "checking underscores for F77 objects..."

rm -rf $TEST_DIR ; mkdir $TEST_DIR ; cd $TEST_DIR
cat >> ./$TEST_NAME.f <<EOF
      PROGRAM hello 
100   format(a,f10.4)
      INCLUDE 'mpif.h'
      INTEGER rank, size, ierr
      INTEGER i
      CALL MPI_INIT( ierr )
      call MPI_Barrier(MPI_COMM_WORLD, ierr)
      call PMPI_Barrier(MPI_COMM_WORLD, ierr)
      CALL MPI_FINALIZE(ierr)
      END
EOF
cat >> ./run <<EOF
#!/bin/bash 

PRE="0"
POST="0"
$MPIF77 -c $FFLAGS $LDFLAGS ./${TEST_NAME}.f -o ./${TEST_NAME}.o
OBJ_FILE=./${TEST_NAME}.o
if test -f \$OBJ_FILE ; 
then 
 MPI_NAME=\$(nm \$OBJ_FILE  | grep mpi_init | grep -vi pmpi | awk '{ print \$[2] }' )
 if test \$(echo \$MPI_NAME | grep -c '_mpi') == "1" ; then
  PRE="1"
 fi 
 if test \$(echo \$MPI_NAME | grep -c '__mpi') == "1" ; then
  PRE="2"
 fi 
 if test \$(echo \$MPI_NAME | grep -c 'init_') == "1" ; then
  POST="1"
 fi 
 if test \$(echo \$MPI_NAME | grep -c 'init__') == "1" ; then
  POST="2"
 fi 
 echo "\$PRE \$POST"
 exit 0
else 
 exit 1
fi
EOF

chmod +x ./run
echo "#test $TEST_NAME start {" >> $CONFIG_LOG
cat ./run >> $CONFIG_LOG
FUNDERSCORES_PRE=$(./run | awk '{print $[1]}')
FUNDERSCORES_POST=$(./run | awk '{print $[2]}')

#echo "PER = ${FUNDERSCORES_PRE}"
#echo "POST= ${FUNDERSCORES_POST}"

FUNDERSCORE="";

if test "0" == "1" ; then
	if  test "$FUNDERSCORES_PRE" == "1" ; then  
# 		echo -n "pre1," 
 		FUNDERSCORE="$FUNDERSCORE -funderscore_pre ";
	elif test "$FUNDERSCORES_PRE" == "2" ; then  
# 		echo -n "pre2," 
 		FUNDERSCORE="$FUNDERSCORE -funderscore_pre -funderscore_pre";
	fi
fi

if  test "$FUNDERSCORES_POST" == "0" ; then  
#	echo "none"
	true
elif  test "$FUNDERSCORES_POST" == "1" ; then  
#	echo "1" 
 	FUNDERSCORE="$FUNDERSCORE -funderscore_post ";
elif  test "$FUNDERSCORES_POST" == "2" ; then  
# 	echo "2" 
 	FUNDERSCORE="$FUNDERSCORE -funderscore_post -funderscore_post";
else
 	echo "unknown"
 	echo "see $CONFIG_LOG for compiler errors."
 	exit 1
fi
## }
fi
# endif MPIF77 exists

AC_SUBST(IPM_FUNDERSCORE, $FUNDERSCORE)
AC_MSG_RESULT([$FUNDERSCORE])

cd $CWD
])
