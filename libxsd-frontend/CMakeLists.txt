cmake_minimum_required(VERSION 3.1)
project(xsd-frontend)

message("configuring XSD-frontend...")

if (NOT XERCES_LIBRARY_PATH)
  message(FATAL_ERROR "Xerces library is not found")
endif()

if (NOT XERCES_INCLUDE_DIR)
  message(FATAL_ERROR "Xerces headers are not found")
endif()

message("using xerces library: " ${XERCES_LIBRARY_PATH})
message("using xerces headers from: " ${XERCES_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../dist/lib)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fopenmp")

link_directories(
    ${PROJECT_SOURCE_DIR}/../dist/lib
  )

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../libcutl
    ${XERCES_INCLUDE_DIR})
  
file(GLOB_RECURSE HEADERS
    "xsd-frontend/*.hxx"
    "xsd-frontend/*.h")

file(GLOB_RECURSE SOURCES
    "xsd-frontend/*.cxx"
    "xsd-frontend/*.c")

add_library(xsd-frontend STATIC ${SOURCES} ${HEADERS})
  
target_link_libraries(xsd-frontend
    cutl
    ${XERCES_LIBRARY_PATH})

set_target_properties(xsd-frontend PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../dist/lib)


