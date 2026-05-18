cmake_minimum_required(VERSION 3.31)
project(hash_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
        hash_core STATIC
        src/sha/sha1.cpp
        src/sha/md5.cpp
)
target_include_directories(hash_core PUBLIC src/sha/include)

add_executable(hash_cli src/app/main.cpp)
target_link_libraries(hash_cli PRIVATE hash_core)
target_compile_options(hash_cli PRIVATE -Wall -Wextra -pedantic)

add_subdirectory(src/lib EXCLUDE_FROM_ALL)

