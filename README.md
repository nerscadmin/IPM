# IPM

Integrated Performance Monitoring for HPC

## Quick start

IPM uses the common GNU autotools build system for compilation. As such, a
typical install of IPM from this repository must begin by generating the
autoconf `configure` script via our provided `bootstrap.sh` script:

    ./bootstrap.sh

Once this is done, you should be able to run

    ./configure
    make
    make install

As usual, you will likely want to examine key options available for the
`configure` script, which can be viewed by running `./configure --help`. For
example, you can enable support for hardware performance counter collection via
PAPI by specifying the PAPI installation path using the `--with-papi=<path>`
option. You may also need to specify your MPI compiler wrappers via the `MPICC`
and `MPIFC` variables - see `./configure --help` for details.

Once you have built and installed IPM, using it to profile your application
should be fairly straightforward. Suppose that `$PREFIX` is the installation
prefix you used when installing IPM. In this case, you can enable profiling by
appending

    -L$PREFIX/lib -lipm

to your link line (making sure it is the last argument). If you are linking an
application that uses the Fortran bindings to MPI, then you will also need to
add `-lipmf`, making sure that it comes before `-lipm`:

    -L$PREFIX/lib -lipmf -lipm

As usual, if you are using dynamic linking and `$PREFIX/lib` has not been added
to your `$LD_LIBRARY_PATH`, then you will likely also want to pass the former
to the linker as an `-rpath`.

**Note**: The `master` branch of IPM is in general usable, but should be
considered development software. We strongly recommend that for production use
you build a tagged release of IPM. The most recent release is `2.0.6`.

### Examples

Suppose that you are building a pure C MPI code contained in a single source
file that you would like to profile with IPM, and that `mpicc` is the MPI C
compiler wrapper for your system. In this case, you could simply run:

    mpicc my_code.c -o my_code.x -L$PREFIX/lib -lipm

to produce an IPM instrumented executable.
 
Now suppose that you are building a mixed C and Fortran MPI code that you would
like to profile with IPM, and that `mpicc` and `mpifort` are the MPI C and
Fortran compiler wrappers for your system. In this case, you could use:

    mpicc -c my_c_routines.c -o my_c_routines.o
    mpifort -c my_fort_routines.f90 -o my_fort_routines.o
    mpifort my_c_routines.o my_fort_routines.o -o my_code.x -L$PREFIX/lib -lipmf -lipm


## About IPM

**IPM is a portable profiling infrastructure for parallel codes.** It provides
a low-overhead profile of application performance and resource utilization in a
parallel program. Communication, computation, and IO are the primary focus.
While the design scope targets production computing in HPC centers, IPM has
found use in application development, performance debugging and parallel
computing education. The level of detail is selectable at runtime and presented
through a variety of text and web reports.

IPM has extremely low overhead, is **scalable** and **easy to use** requiring
no source code modification. It runs on Cray XT/XE, IBM Blue Gene, most Linux
clusters using MPICH/OPENMPI, SGI Altix and some NEC machines. IPM is available
under an Open Source software license (LGPL). It is currently installed on
several [Teragrid][], [Department of Energy][], and other supercomputing
resources. 

[teragrid]: https://www.teragrid.org/
[department of energy]: http://www.nersc.gov

IPM brings together several types of information important to developers and
users of parallel HPC codes. The information is gathered in a way the tries to
minimize the impact on the running code, maintaining a small fixed memory
footprint and using minimal amounts of CPU. When the profile is generated the
data from individual tasks is aggregated in a scalable way.


IPM is modular. You can measure just what you want. In addition to the core
timings and benchmarking data, IPM currently delivers the following modules:

  *   **MPI:** Communication topology and statistics for each MPI call and buffer size. 
  *   **HPM:** FLOPs and such via PAPI on-chip event counters.
  *   **OpenMP:** thread level details about load imbalance. 
  *   **Memory:** memory high watermark, getrusage, sbrk. 
  *   **Switch:**Communication volume and packet loss.
  *   **File I/O:** Data written to and read from disk.
  *   **GPU:** Memory copied in/out of the GPU and time in kernels.
  *   **Power:** Joules of energy consumed by the app. 
  
The 'integrated' in IPM is multi-faceted. It refers to combining the above
information together through a common interface and also the integration of the
records from all the parallel tasks into a single report. At a high level we
seek to integrate together the information useful to all stakeholders in HPC
into a common interface that enables a deeper understanding. This includes
application developers, science teams using applications, HPC managers, and
system architects.

## Known issues

The following are known issues affecting the current `master` branch of IPM:

##### Incorrect MPI message sizes in certain circumstances

1. IPM may report incorrect message sizes for the family of `MPI_Wait`
and `MPI_Test` functions. This is because IPM indiscriminately
extracts the message size from the status object of every single
`MPI_Wait` and `MPI_Test` call. However, there are at least 3
situations when the status object is either undefined or partially
defined.
   *  **The request handle is from a non-blocking send**
      >"The fields in a status object returned by a call to MPI_WAIT,
      MPI_TEST, or any of the other derived functions
      (MPI_{TEST|WAIT}{ALL|SOME|ANY}), where the request corresponds
      to a send call, are undefined, with two exceptions: The error
      status field will contain valid information if the wait or test
      call returned with MPI_ERR_IN_STATUS ; and the returned status
      can be queried by the call MPI_TEST_CANCELLED." [MPI standard
      v3.1 page 52]

   *  **The request handle is from a non-blocking collective call**
      >"Upon returning from a completion call in which a nonblocking
      collective operation completes, the MPI_ERROR field in the
      associated status object is set appropriately, see Section
      3.2.5. The values of the MPI_SOURCE and MPI_TAG fields are
      undefined." [MPI standard v3.1 page 197]

   *  **The request handle is from an RMA operation**
      >"Upon returning from a completion call in which an RMA
      operation completes, the MPI_ERROR field in the associated
      status object is set appropriately (see Section 3.2.5). All
      other fields of status and the results of status query functions
      (e.g., MPI_GET_COUNT) are undefined." [MPI standard v3.1 page
      430]

   The MPI implementation may still provide the message size for these
special cases but it is not required by the standard. For example, we
have found that `openmpi-1.8.1` initializes status data corresponding
to send requests, while `mpich-2.1.5` and `mpich-3.3a2` do not.

2. IPM will not collect the message size for an MPI function that is
passed `MPI_STATUS_IGNORE` or `MPI_STATUSES_IGNORE`. This affects
receive, probe, wait and test functions. The only receive function
that is not affected by this issue is `MPI_Irecv`.

3. IPM may report incorrect message sizes for `MPI_Irecv` because the
message size is calculated from the receive count and datatype
arguments. This may be larger than the eventual message because it is
legitimate for the sender to send a shorter message.

   Key takeaway: IPM is only expected to collect correct message sizes
for MPI send and MPI collective functions. The message size data for
other MPI functions should be ignored unless you know that your
application is not affected by one of the above issues.
