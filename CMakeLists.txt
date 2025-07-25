cmake_minimum_required(VERSION 3.10)
project(BDMan)

# Tell CMake where to find your source files
add_executable(BDMan 
    BDMan/BDMan.cpp 
    BDMan/BDMan.h
)

set_property(TARGET BDMan PROPERTY CXX_STANDARD 20)

# Find CURL
find_package(CURL REQUIRED)
target_link_libraries(BDMan PRIVATE CURL::libcurl)

# Indicators is header-only, so just add include dirs
find_path(INDICATORS_INCLUDE_DIR indicators/block_progress_bar.hpp)
target_include_directories(BDMan PRIVATE ${INDICATORS_INCLUDE_DIR})