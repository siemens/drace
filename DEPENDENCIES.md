# Dependencies

This project uses three kinds of dependencies:

- External
- Git Submodules
- Internal

## External Dependencies

With external dependencies we refer to software components that are not shipped in this repository, but which are necessary to build or run DRace.

- CMake > 3.8
- [DynamoRIO](https://github.com/DynamoRIO/dynamorio) 8.0.x (use weekly releases)
- C++11 / C99 Compiler (e.g msvc)

## Git Submodules

The source of these dependencies are not stored inside the repository, but included using git submodules.
All submodules are located in the `vendor` directory.

### DRace

**Mandatory:**

- [jtilly/inih](https://github.com/jtilly/inih)
- [HowardHinnant/date](https://github.com/HowardHinnant/date)
- [muellan/clipp](https://github.com/muellan/clipp)

**Optional:**

- [leethomason/tinyxml2](https://github.com/leethomason/tinyxml2)
- [google/googletest](https://github.com/google/googletest)
- [google/benchmark](https://github.com/google/benchmark)
- [greq7mdp/parallel-hashmap](https://github.com/greg7mdp/parallel-hashmap)

#### Detectors

**Fasttrack**

- Boost > 1.65

### Managed Symbol Resolver (MSR)

**Mandatory:**

- [gabime/spdlog](https://github.com/gabime/spdlog)
- [greq7mdp/sparsepp](https://github.com/greg7mdp/sparsepp)

### DRace GUI

**Mandatory:**

- [Qt5](https://doc.qt.io/qt-5/)
- [Boost](https://www.boost.org/) > 1.65
- [nlohmann/json](https://github.com/nlohmann/json)

## Internal Dependencies

These components are adapted or modified for DRace and shipped with the repository.
The exact license wording is found either in the source file directly, or in a dedicated license file in the directory.

| Name           | Location                           | License      | Comment                                                     |
| -------------- | ---------------------------------- | ------------ | ----------------------------------------------------------- |
| LLVM-Tsan      | `vendor/tsan`                      | BSD-Like/MIT | a customized version is included in binary format           |
| CoreCLR Header | `ManagedResolver/include/prebuild` | MIT          | prebuild version of MIDL files (cannot be build on windows) |
| Ringbuffer     | `common/ipc/ringbuffer.cpp`        | CC0          | generic ring buffer implementation for embedded targets     |
| LossyCounting  | `drace-client/include/lcm`         | BSD 3-Clause | fast lossy-counting model implementation                    |
| git-watcher    | `CMakeExt/git_watcher.cmake`       | MIT          | burn in git commit hash into binary                         |
