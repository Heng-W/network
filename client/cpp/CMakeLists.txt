cmake_minimum_required(VERSION 2.8)

project(net)

set(CXX_FLAGS "-std=c++11 -Wall")

set(TARGET "net_client")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX $ENV{HOME}/local/net_client CACHE PATH "Default install prefix" FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CXX_FLAGS} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CXX_FLAGS} -O2 -DNDEBUG")

add_subdirectory(src/net)
add_subdirectory(src/util)

add_library(${TARGET} STATIC $<TARGET_OBJECTS:net> $<TARGET_OBJECTS:util>)

install(TARGETS ${TARGET} ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

