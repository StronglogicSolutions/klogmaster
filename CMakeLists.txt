cmake_minimum_required(VERSION 3.20)
project(klogmaster)

set(CMAKE_CXX_STANDARD 20)

find_package(fmt REQUIRED)

# Add the httplib header-only library
include_directories(
  third_party/httplib
  third_party/kutils/include
  third_party/klogger/src
)

# Add the source files
set(SOURCE_FILES
    src/main.cpp
    src/server.hpp
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third_party/klogger/build/libklog.a)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)