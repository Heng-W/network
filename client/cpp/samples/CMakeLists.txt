cmake_minimum_required(VERSION 2.8)

project(net_sample)

set(CXX_FLAGS "-std=c++11 -Wall")

if(NOT PACKAGE_DIR)
    set(PACKAGE_DIR $ENV{HOME}/local/net_client)
endif()

if(WIN32)
    MESSAGE(STATUS "Windows")
    if(MINGW)
        set(LIB_DIRS ${PACKAGE_DIR}/lib)
        set(LINK_LIBS net_client ws2_32)
    endif()
elseif(UNIX)
    MESSAGE(STATUS "UNIX-like OS")
    set(CXX_FLAGS "${CXX_FLAGS} -pthread")
    set(LIB_DIRS "${PACKAGE_DIR}/lib")
    set(LINK_LIBS net_client)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CXX_FLAGS} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CXX_FLAGS} -O2 -DNDEBUG")


file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

foreach(SRC ${SRCS})
    string(REPLACE ".cpp" "" target ${SRC})
    include_directories(${PACKAGE_DIR}/include)
    link_directories(${LIB_DIRS})
    add_executable(${target} ${SRC})
    target_link_libraries(${target} ${LINK_LIBS})	
endforeach(SRC)

