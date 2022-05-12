cmake_minimum_required (VERSION 3.21)

project (libwimer CXX)

set(_sources lib.cpp
             wimer.h)

add_library(libwimer STATIC ${_sources})

# the dynamic library with auto-generated export declarations
# add_library(libwimer SHARED ${_sources})
# include(GenerateExportHeader)
# generate_export_header(libwimer)
target_include_directories(libwimer PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
