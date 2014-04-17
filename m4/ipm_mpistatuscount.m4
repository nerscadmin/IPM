

AC_DEFUN([AX_IPM_MPISTATUSCOUNT], [
AC_MSG_CHECKING([checking MPI_STATUS_COUNT])

CWD=$PWD

TEST_DIR="$PWD/.test"
CONFIG_LOG=config.test.log

MPI_STATUS_COUNT=""

for tag in val1 count _count size _ucount count_lo st_count; do
	TEST_NAME="simplec_MPI_STATUS_COUNT"
#	echo -n "$tag"
	rm -rf $TEST_DIR ; mkdir $TEST_DIR ; cd $TEST_DIR
	cat >> ./$TEST_NAME.c <<EOF
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
int main(int argc, char *argv[]) {
 MPI_Status s;
 s.$tag = 0;
 return 0;
}
EOF

	cat >> ./run <<EOF
#!/bin/sh -x
$MPICC $CFLAGS $LDFLAGS ./$TEST_NAME.c -o ./$TEST_NAME
if test -x ./$TEST_NAME ; then 
 exit 0
else 
 exit 1
fi
EOF

	chmod +x ./run
	echo "#test $TEST_NAME start {" >> $CONFIG_LOG
	./run >> $CONFIG_LOG 2>&1
	if test $? == 0  ; then 
# 		echo "yes"
 		MPI_STATUS_COUNT=$tag
 		break
#	else
# 		echo "no"
	fi
	cd $BUILD_ROOT
done

if test "x$MPI_STATUS_COUNT" == "x" ; then
	AC_MSG_RESULT( unknown )
	exit 1
else 
 	AC_MSG_RESULT( $MPI_STATUS_COUNT )
 	AC_SUBST(IPM_MPISTATUSCOUNT, $MPI_STATUS_COUNT)
fi

cd $CWD

])
