cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(RadMapAnalysis)

#set (CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
set(CMAKE_MODULE_PATH $ENV{ROOTSYS}/etc/cmake)
find_package(ROOT REQUIRED COMPONENTS MathCore RIO Hist Tree Net)
include_directories(${ROOT_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_CXX_FLAGS})

set(SRCS
   src/Dummy.cxx
   )

set(HEADERS
   include/Dummy.h
   )

ROOT_GENERATE_DICTIONARY(G__RMA ${HEADERS} LINKDEF include/LinkDef.h)
add_library(RadMapAna SHARED ${SRCS} G__RMA)
target_link_libraries(RadMapAna ${ROOT_LIBRARIES}) 

# Create a main program using the library
add_executable(runDummy macros/runDummy.C)
target_link_libraries(runDummy RadMapAna)
