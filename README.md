# IPM

Integrated Performance Monitoring for HPC

## Quick start

IPM uses the common GNU autotools build system for compilation. As such, a
typical install of IPM from this repository must begin by generating the
autoconf `configure` script via:

    autoreconf -f -i

Once this is done, you should be able to run

    ./configure
    make
    make install

As usual, you will likely want to examine key options available for the
`configure` script, which can be viewed by running `./configure --help`. For
example, you can enable support for hardware performance counter collection via
PAPI by specifying the PAPI installation path using the `--with-papi=<path>`
option.

**Note**: The `master` branch of IPM is in general usable, but should be
considered development software. We strongly recommend that for production use
you build a tagged release of IPM. The most recent release is `2.0.3`.

## About IPM

**IPM is a portable profiling infrastructure for parallel codes.** It provides a low-overhead profile of application performance and resource utilization in a parallel program. Communication, computation, and IO are the primary focus. While the design scope targets production computing in HPC centers, IPM has found use in application development, performance debugging and parallel computing education. The level of detail is selectable at runtime and presented through a variety of text and web reports.

IPM has extremely low overhead, is **scalable** and **easy to use** requiring no source code modification. It runs on Cray XT/XE, IBM Blue Gene, most Linux clusters using MPICH/OPENMPI, SGI Altix and some NEC machines. IPM is available under an Open Source software license (LGPL). It is currently installed on several [Teragrid][], [Department of Energy][], and other supercomputing resources. 

[teragrid]: https://www.teragrid.org/
[department of energy]: http://www.nersc.gov

IPM brings together several types of information important to developers and users of parallel HPC codes. The information is gathered in a way the tries to minimize the impact on the running code, maintaining a small fixed memory footprint and using minimal amounts of CPU. When the profile is generated the data from individual tasks is aggregated in a scalable way.


IPM is modular. You can measure just what you want. In addition to the core timings and benchmarking data, IPM currently delivers the following modules:

  *   **MPI:** Communication topology and statistics for each MPI call and buffer size. 
  *   **HPM:** FLOPs and such via PAPI on-chip event counters.
  *   **OpenMP:** thread level details about load imbalance. 
  *   **Memory:** memory high watermark, getrusage, sbrk. 
  *   **Switch:**Communication volume and packet loss.
  *   **File I/O:** Data written to and read from disk.
  *   **GPU:** Memory copied in/out of the GPU and time in kernels.
  *   **Power:** Joules of energy consumed by the app. 
  
The 'integrated' in IPM is multi-faceted. It refers to combining the above information together through a common interface and also the integration of the records from all the parallel tasks into a single report. At a high level we seek to integrate together the information useful to all stakeholders in HPC into a common interface that enables a deeper understanding. This includes application developers, science teams using applications, HPC managers, and system architects.

## Known issues

The following are known issues affecting the current `master` branch of IPM:
* `VPATH` builds are not yet supported
* Depending on the compiler wrappers associated with your MPI implementation,
  you may need to use the MPI fortran compiler (e.g. `mpif90`) for linking your
  program when IPM is build from commit `8074f3f` or later
