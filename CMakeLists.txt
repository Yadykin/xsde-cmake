cmake_minimum_required(VERSION 3.1)

project(xsde-cmake)

# this makes this variables visible for cmake-gui and filled with default value first time
set(XERCES_LIBRARY_PATH XERCES_LIBRARY_PATH-NOTFOUND CACHE FILEPATH "Path to xercec-c library")
set(XERCES_INCLUDE_DIR XERCES_INCLUDE_DIR-NOTFOUND CACHE PATH "Path to xerces include dir (that contains subdirs like xinclude, dom, etc)")

if (NOT EXISTS ${XERCES_LIBRARY_PATH})
  message("Checking xerces library...failed")
  set(XERCES_LIBRARY_PATH "XERCES_LIBRARY_PATH-NOTFOUND")
endif()

set (testpath ${XERCES_INCLUDE_DIR}/xercesc/xinclude/XIncludeUtils.hpp)
if (NOT EXISTS ${testpath})
  message("Checking xerces include directory...failed")
  message("Set XERCES_INCLUDE_DIR to directory, that contains <xercesc/xinclude/XIncludeUtils.hpp>")
  set(XERCES_INCLUDE_DIR "XERCES_INCLUDE_DIR-NOTFOUND")
endif()

add_subdirectory (${PROJECT_SOURCE_DIR}/libcutl)
add_subdirectory (${PROJECT_SOURCE_DIR}/libxsd-frontend)
add_subdirectory (${PROJECT_SOURCE_DIR}/xsde/xsde)
add_subdirectory (${PROJECT_SOURCE_DIR}/xsde/libxsde)

