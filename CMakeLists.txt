cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(RadMapAnalysis)

#set (CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
set(CMAKE_MODULE_PATH $ENV{ROOTSYS}/etc/cmake)
find_package(ROOT REQUIRED COMPONENTS MathCore RIO Hist Tree Net)
#include_directories(${ROOT_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_CXX_FLAGS})


set(SRCS
   src/Dummy.cxx
   src/Signal.cxx
   )

set(HEADERS
   include/Dummy.h
   include/Signal.h
   )

ROOT_GENERATE_DICTIONARY(G__RMA ${HEADERS} LINKDEF include/LinkDef.h)
add_library(RadMapAna SHARED ${SRCS} G__RMA)
target_link_libraries(RadMapAna ${ROOT_LIBRARIES}) 

# Create a main program using the library

set(EXE_SRCS
    macros/runDummy.C)

set(EXE_NAMES
    runDummy)

list(LENGTH EXE_SRCS count)
math(EXPR count "${count}-1")
foreach(i RANGE ${count})
  list(GET EXE_SRCS ${i} src)
  list(GET EXE_NAMES ${i} name)
  add_executable(${name} ${src})
  set_target_properties(${name}
                      PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
  target_link_libraries(${name} RadMapAna)
endforeach()
