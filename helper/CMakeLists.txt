
set(HELPER_SOURCES "src/main.cpp")

add_executable("helper" ${HELPER_SOURCES})

set_target_properties("helper" PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)


if(NOT TARGET Threads::Threads)
    find_package(Threads REQUIRED)
endif()

target_include_directories("helper" PUBLIC "include")


target_link_libraries("helper" PUBLIC Threads::Threads)
