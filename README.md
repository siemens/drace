# DRace

[![REUSE status](https://api.reuse.software/badge/github.com/siemens/drace)](https://api.reuse.software/info/github.com/siemens/drace)
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/2553/badge)](https://bestpractices.coreinfrastructure.org/projects/2553)

DRace is a data-race detector for Windows applications which uses DynamoRIO
to dynamically instrument a binary at runtime.
It does not require any preparations like instrumentation of the binary to check.
While the detector should work with all binaries that use the Windows synchronization API,
we focus on applications written in C and C++.
Experimental support for hybrid applications containing native and Dotnet parts
is implemented as well.

For best results, we recommend to provide debug symbols of the
application under test along with the binary.
Without this information, callstacks cannot be fully symbolized (exported functions only)
and user-level synchronization cannot be detected.

## Dependencies

### Runtime

When using the pre-build releases, only DynamoRIO is required:

- [DynamoRIO](https://github.com/DynamoRIO/dynamorio) 8.0.x (use weekly releases)
- [Debugging Tools for Windows](https://docs.microsoft.com/de-de/windows-hardware/drivers/debugger/debugger-download-tools) for symbol lookup in .Net runtime libraries (only required for .Net)

## Using the DRace Race Detector

**Run the detector as follows**

```bash
drrun.exe -no_follow_children -c drace-client.dll <detector parameter> -- application.exe <app parameter>
# see limitations for -no_follow_children option
```

**Command Line Options**

```
SYNOPSIS
        drace-client.dll [-c <config>] [-d <detector> [<detector-options>]...] [-s <sample-rate>]
                         [-i <instr-rate>] [--lossy [--lossy-flush]] [--excl-traces] [--excl-stack]
                         [--excl-master] [--stacksz <stacksz>] [--no-annotations] [--delay-syms]
                         [--suplevel <level>] [--sup-races <sup-races>] [--xml-file <filename>]
                         [--out-file <filename>] [--logfile <filename>] [--extctrl] [--brkonrace]
                         [--stats] [--version] [-h] [--heap-only]
OPTIONS
        DRace Options
            -c, --config <config>
                    config file (default: drace.ini)
            -d, --detector <detector> <detector-options>
                    race detector (default: tsan)
            sampling options
                -s, --sample-rate <sample-rate>
                    sample each nth instruction (default: no sampling)
                -i, --instr-rate <instr-rate>
                    instrument each nth instruction (default: no sampling, 0: no instrumentation)
            analysis scope
                --lossy
                    dynamically exclude fragments using lossy counting
                --lossy-flush
                    de-instrument flushed segments (only with --lossy)
                --excl-traces
                    exclude dynamorio traces
                --excl-stack
                    exclude stack accesses
                --excl-master
                    exclude first thread
            --stacksz <stacksz>
                    size of callstack used for race-detection (must be in [1,31], default: 31)
            --no-annotations
                    disable code annotation support
            --delay-syms
                    perform symbol lookup after application shutdown
            --suplevel <level>
                    suppress similar races (0=detector-default, 1=unique top-of-callstack entry,
                    default: 1)
            --sup-races <sup-races>
                    race suppression file (default: race_suppressions.txt)
            data race reporting
                --xml-file, -x <filename>
                    log races in valkyries xml format in this file
                --out-file, -o <filename>
                    log races in human readable format in this file
            --logfile, -l <filename>
                    write all logs to this file (can be null, stdout, stderr, or filename)
            --extctrl
                    use second process for symbol lookup and state-controlling (required for Dotnet)
            --brkonrace
                    abort execution after first race is found (for testing purpose only)
            --stats display per-thread statistics on thread-exit
            --version
                    display version information
            -h, --usage
                    display help
        Detector Options
            --heap-only
                    only analyze heap memory (not supported currently)
```

### Available Detectors

DRace is shipped with the following detector backends:

- tsan (internal ThreadSanitizer)
- fasttrack (note: experimental)
- dummy (no detection at all)
- printer (print all calls to the detector)

#### tsan

The detector is run along with the application. No further threads are started.

#### fasttrack

An implementation of the FT2 algorithm. Less optimized than tsan, still experimental support only.

#### dummy

This detector does not detect any races. It is there to evaluate the overhead of the other detectors vs the instrumentation overhead.


### Externally Controlling DRace

DRace can be externally controlled from a controller (`msr.exe`) running in a second process.
To set the detector state during runtime, the following keys (committed using `enter`) are available:

```
e        enable detector on all threads
d        disable detector on all threads
s <rate> set sampling rate to 1/x (similar to `-s` in DRace)
```

### Symbol Resolving

DRace requires symbol information for wrapping functions and to resolve stack traces.
For the main functionality of C and C++ only applications, export information is sufficient.
However for additional and more precise race-detection (e.g. C++11, QT), debug information is necessary.

The application searches for this information in the path of the module and in `_NT_SYMBOL_PATH`.
However, only local symbols are searched (non `SRV` parts).

If symbols for system libraries are necessary (e.g. for Dotnet), they have to be downloaded from a symbol server.
Thereto it is useful to set the variable as follows:

```
set _NT_SYMBOL_PATH="c:\\symbolcache\\;SRV*c:\\symbolcache\\*https://msdl.microsoft.com/download/symbols"
```

**Note:**
Downloading symbols is only supported if the [Debugging Tools for Windows](https://docs.microsoft.com/de-de/windows-hardware/drivers/debugger/debugger-download-tools) are installed.
If not, DRace uses the default `dbghelp.dll` which is not bundled with `symsrv.dll` and hence is not able to download symbols.

### Dotnet

For .Net managed code, a second process (MSR) is needed for symbol resolution.
The MSR is started as follows:

```
ManagedResolver\msr.exe [-v for verbose]
```

After it is started, DRace connects to MSR using shared memory.
The MSR then tries to locate the correct [DAC](https://github.com/dotnet/coreclr/blob/master/Documentation/botr/dac-notes.md)
DLL to resolve managed program counters and symbols.

The output (logs) of the MSR are just for debugging reasons.
The resolved symbols are passed back to DRace and merged with the non-managed ones.

**Note:**
To properly detect dotnet synchronization, pdb symbol information is required.
The pdb files have to perfectly match the used dotnet libraries.
Hence, it is (almost always) mandatory to let the MSR download the symbols.
Thereto, point the `_NT_SYMBOL_PATH` variable to a MS symbol server, as shown one section above.

### Custom Annotations

Custom synchronization logic is supported by annotating the corresponding code sections.
Thereto we provide a header with macros in `drace-client/include/annotations/drace_annotation.h`.
To enable these macros, define `DRACE_ANNOTATION` prior to including the header.

A example on how to use the annotations is provided in `test/mini-apps/annotations/`.

## Testing

The unit test for the detector backends and other components can be executed with ctest:
```
# Unit tests
ctest -j4 -T test --output-on-failure
```

Integration tests for the complete DR-Client can be executed using the following command:
```
# Integration Tests
# Windows
./bin/drace-system-tests.exe --gtest_output="xml:test-system-results.xml"
#Linux
./bin/drace-system-tests --gtest_output="xml:test-system-results.xml"
```

**Note:** Before pushing a commit, please run the integration tests.
Later on, bugs are very tricky to find.

## Build

DRace is build using CMake. The only (mandatory) external dependency is DynamoRIO.
For best compatibility with Windows 10, use the latest available weekly build.
The path to your DynamoRIO installation has to be set using `-DDynamoRIO_DIR`.

If you want to use the drace-gui, you must specify a path to boost and Qt5.

All other dependencies are either internal or included using git submodules.
To clone all submodules of this repository, issue the following commands inside the drace directory:

```
git submodule init
git submodule update --recursive
```

A sample VisualStudio `CMakeSettings.json` is given here:

```
{
  "name": "x64-Release",
  "generator": "Ninja",
  "configurationType": "RelWithDebInfo",
  "inheritEnvironments": [ "msvc_x64_x64" ],
  "buildRoot": "${env.USERPROFILE}\\CMakeBuilds\\${workspaceHash}\\build\\${name}",
  "installRoot": "${env.USERPROFILE}\\CMakeBuilds\\${workspaceHash}\\install\\${name}",
  "cmakeCommandArgs": "-DBUILD_TESTING=1 -DDynamoRIO_DIR=<PATH-TO-DYNAMORIO>/cmake -DBOOST_ROOT=<PATH-TO-BOOST> -DCMAKE_PREFIX_PATH=<PATH-TO-QT>\\msvc2017_64\\lib\\cmake\\Qt5",
  "buildCommandArgs": "-v",
  "ctestCommandArgs": ""
}
```

### Documentation

A doxygen documentation can be generated by building the `doc` target.

### Dependencies

For detailed information on all dependencies, see `DEPENDENCIES.md`.

#### Development

- CMake > 3.8
- C++11 / C99 Compiler
- [DynamoRIO](https://github.com/DynamoRIO/dynamorio) > 8.0.x

#### External Libraries

##### DRace

**Mandatory:**

- [jtilly/inih](https://github.com/jtilly/inih)
- [HowardHinnant/date](https://github.com/HowardHinnant/date)
- [muellan/clipp](https://github.com/muellan/clipp)
- LLVM-Tsan (a customized version is included in binary format)

**Optional:**

- [leethomason/tinyxml2](https://github.com/leethomason/tinyxml2)
- [google/googletest](https://github.com/google/googletest)
- [google/benchmark](https://github.com/google/benchmark)

##### DRaceGUI

**Mandatory:**

- [nlohmann/json](https://github.com/nlohmann/json) 

##### Managed Symbol Resolver (MSR)

**Mandatory:**

- [gabime/spdlog](https://github.com/gabime/spdlog)
- [greq7mdp/sparsepp](https://github.com/greg7mdp/sparsepp)

## Tools

### DRaceGUI

The DRaceGUI is a graphical interface with which one can use DRace without typing a very long command into the Powershell. This is especially useful for users which use DRace for the first time.

For more information have a look in [here](tools/DRaceGUI)

### ReportConverter

With the ReportConverter an HTML report generator was added to the project. By using the `ReportConverter.py` script (or the ReportConverter.exe, which is very slow) one can generate an HTML report from the generated drace XML report.

For more information have a look in [here](tools/ReportConverter)

### Standalone

The DRACE_ENABLE_RUNTIME CMake flag can be set to ```false```, if one only wants  to build the standalone components of the DRace project. Can be used as a standalone data race detector backend for more general problems.

Standalone Components:

- drace::detector::Fasttrack (Standalone Version)
- Binary Decoder

## Limitations

### All Detectors

- The size of variables is not considered when detecting races (Only races of variables with the same (base) address are detected. Potential overlaps of variables are ignored.).
- Finished threads are deleted from the analysis. No race detection of already finished threads.
- When using powershell, debug outputs from the detector backend are lost. Use e.g. the git bash instead.

### TSAN

- TSAN can only be started once, as the cleanup is not fully working
- `no_follow_children`: Due to the TSAN limitation, drace can only analyze a single process. This process is the initially started one.

### Fasttrack (Standalone)

- On 32 Bit architectures, only 16 bits are used to store a thread. This could theoretically cause problems as Windows TIDs are 32 bits (DWORD). If two threads would have the same last 16 bits, they would be considered as the same frame.

## Licensing

DRace is primarily licensed under the terms of the MIT license.

Each of its source code files contains a license declaration in its header. Whenever a file is provided under an additional or different license than MIT, this is stated in the file header.
Any file that may lack such a header has to be considered licensed under MIT (default license).

If two licenses are specified in a file header, you are free to pick the one that suits best your particular use case.
You can also continue to use the file under the dual license.
When choosing only one, remove the reference to the other from the file header.

### External Resources

Most external resources are located in the `vendor` directory.
For licensing information regarding these components, we refer to the information bundled with the individual resource.

### License Header Format

We use the [REUSE](https://reuse.software/practices/) format for license and copyright information.

```
/*
 * DRace, a dynamic data race detector
 *
 * Copyright <YEAR> <COPYRIGHT HOLDER>
 *
 * SPDX-License-Identifier: MIT
 */
```

## Citing DRace

A publicly available fulltext of the master's thesis can be found here: [High Performance Dynamic Threading Analysis for Hybrid Applications](https://epub.ub.uni-muenchen.de/60621/)

To cite DRace, please reference:

```
F. Mößbauer. "High Performance Dynamic Threading Analysis for Hybrid Applications", Master Thesis, Faculty of Mathematics, Computer Science and Statistics, Ludwig-Maximilians-Universität München (2019).
```

**BibTex**
```
@misc{moes19,
           title = {High Performance Dynamic Threading Analysis for Hybrid Applications},
         keyword = {Concurrency Bugs; Race Condition; Program Analysis; Binary Instrumentation; Sampling; Managed Applications},
        abstract = {Verifying the correctness of multithreaded programs is a challenging task due to errors that occur sporadically. Testing, the most important verification method for decades, has proven to be ineffective in this context. On the other hand, data race detectors are very successful in finding concurrency bugs that occur due to missing synchronization. However, those tools introduce a huge runtime overhead and therefore are not applicable to the analysis of real-time applications. Additionally, hybrid binaries consisting of Dotnet and native components are beyond the scope of many data race detectors.
In this thesis, we present a novel approach for a dynamic low-overhead data race detector. We contribute a set of fine-grained tuning techniques based on sampling and scoping. These are evaluated on real-world applications, demonstrating that the runtime overhead is reduced while still maintaining a good detection accuracy. Further, we present a proof of concept for hybrid applications and show that data races in managed Dotnet code are detectable by analyzing the
application on the binary layer. The approaches presented in this thesis are implemented in the open-source tool DRace.},
            year = {2019},
          author = {Felix M\"o\ssbauer},
             url = {http://nbn-resolving.de/urn/resolver.pl?urn=nbn:de:bvb:19-epub-60621-8}
}
```

