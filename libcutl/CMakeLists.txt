cmake_minimum_required(VERSION 3.1)
project(cutl)

message("configuring cutl...")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../dist/lib)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fopenmp")

include_directories(
    ${PROJECT_SOURCE_DIR})
  
file(GLOB_RECURSE HEADERS
    "cutl/*.hxx"
    "cutl/*.h")

file(GLOB_RECURSE SOURCES
    "cutl/*.cxx"
    "cutl/*.c")

add_library(cutl STATIC ${SOURCES} ${HEADERS})

set_target_properties(cutl PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../dist/lib)

