

set(PROF_SOURCES "src/prof.cpp")

add_executable("profiling" ${PROF_SOURCES})


target_include_directories("profiling"
                PUBLIC "include"
                PUBLIC "${Boost_INCLUDE_DIR}")

find_package(easy_profiler REQUIRED)
                   
target_link_libraries("profiling" easy_profiler
"drace-common" "parallel-hashmap" "drace.detector.fasttrack.generic")

set_target_properties(
  "profiling" PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
