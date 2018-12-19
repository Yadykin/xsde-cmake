cmake_minimum_required(VERSION 3.1)
project(xsdeCompiler)

message("configuring xsde compiler...")

if (NOT XERCES_LIBRARY_PATH)
  message(FATAL_ERROR "Xerces library is not found")
endif()

if (NOT XERCES_INCLUDE_DIR)
  message(FATAL_ERROR "Xerces headers are not found")
endif()

set(XSDE_COMPILER_INSTALL_DIR CACHE PATH "Path where xsdeCompiler should be installed")

message("using xerces library: " ${XERCES_LIBRARY_PATH})
message("using xerces headers from: " ${XERCES_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../dist/bin)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fopenmp")

link_directories(
    ${PROJECT_SOURCE_DIR}/../../dist/lib
  )

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../../libcutl
    ${PROJECT_SOURCE_DIR}/../../libxsd-frontend
    ${XERCES_INCLUDE_DIR})
  
file(GLOB_RECURSE HEADERS
    "*.hxx"
    "*.h")

file(GLOB_RECURSE SOURCES
    "*.cxx"
    "*.c")

add_executable(xsdeCompiler ${SOURCES} ${HEADERS})
  
target_link_libraries(xsdeCompiler
    cutl
    xsd-frontend
    ${XERCES_LIBRARY_PATH})

set_target_properties(xsdeCompiler PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../dist/bin)

if (XSDE_COMPILER_INSTALL_DIR)
  install(TARGETS xsdeCompiler RUNTIME DESTINATION ${XSDE_COMPILER_INSTALL_DIR})
endif()
