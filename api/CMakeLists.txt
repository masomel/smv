cmake_minimum_required(VERSION 2.6)
project(smv)

#set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_C_FLAGS "-O0 -g")

if(APPLE)
	set(CMAKE_OSX_ARCHITECTURES i386 x86_64)
endif()
##set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -g")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g")
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(smv_lib SHARED smv_lib.c)
add_library(memdom_lib SHARED memdom_lib.c mem_benchmarking_util.c)
add_library(kernel_comm SHARED kernel_comm.c)
add_library(smv SHARED smv_lib.c memdom_lib.c mem_benchmarking_util.c kernel_comm.c)

target_link_libraries(smv smv_lib memdom_lib kernel_comm pthread) 

