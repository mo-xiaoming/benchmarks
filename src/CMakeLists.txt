find_package(benchmark REQUIRED)

add_executable(benchmaps main.cpp)
target_link_libraries(benchmaps PRIVATE project_defaults benchmark::benchmark benchmark::benchmark_main)

