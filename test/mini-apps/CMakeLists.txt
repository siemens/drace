# build all mini-applications

# put all sample applications into dedicated folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/samples")
set(DRACE_RUNTIME_DEST "${DRACE_RUNTIME_DEST}/samples")

add_subdirectory("concurrent-inc")
add_subdirectory("inc-mutex")
add_subdirectory("empty-main")
add_subdirectory("single-thread")
add_subdirectory("annotations")
add_subdirectory("atomics")
add_subdirectory("sampler")
add_subdirectory("memchecker")
add_subdirectory("segfault")

if(WIN32)
    add_subdirectory("lock-kinds")
    # CSharp examples
    add_subdirectory("cs-sync")
endif()
