set (SOURCES "ft_benchmark.cpp")

set (TARGET_NAME "ft_benchmark")

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} Threads::Threads)
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17 CX_STANDARD_REQUIRED ON)
