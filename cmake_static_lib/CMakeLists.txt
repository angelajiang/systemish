cmake_minimum_required(VERSION 3.5)
project(eRPC)

set(CMAKE_CXX_COMPILER clang++-6.0)
#set(CMAKE_CXX_COMPILER g++-7)

add_definitions(-std=c++11)

add_definitions(-Wall -Wextra -Werror -pedantic)
add_definitions(-Wsign-conversion)
add_definitions(-Wold-style-cast)

#add_definitions(-Wno-unused-function)
#add_definitions(-Wno-nested-anon-types)
add_definitions(-march=native)
add_definitions(-g)
add_definitions(-O3)


include_directories(${CMAKE_SOURCE_DIR}/src)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

set(SOURCES
  src/foo.cc)

add_library(foolib ${SOURCES})

add_executable(sample sample.cc)
target_link_libraries(sample foolib)
