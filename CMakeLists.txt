cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(RadMapAnalysis)

#set (CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
set(CMAKE_MODULE_PATH $ENV{ROOTSYS}/etc/cmake)
find_package(ROOT REQUIRED COMPONENTS MathCore RIO Hist Tree Net)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_CXX_FLAGS})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)


set(SRCS
   src/Dummy.cxx
   src/Signal.cxx
   src/Fiber.cxx
   src/EventBase.cxx
   src/CTSEvent.cxx
   src/Module.cxx
   src/Utility.cxx
   src/Cluster.cxx
   src/CTSEventClusters.cxx
   src/Track.cxx
   src/Tracker.cxx
   src/Clusterer.cxx
   )

set(HEADERS
   include/Dummy.h
   include/Signal.h
   include/Fiber.h
   include/EventBase.h
   include/CTSEvent.h
   include/Module.h
   include/Utility.h
   include/Cluster.h
   include/CTSEventClusters.h
   include/Track.h
   include/Tracker.h
   include/Clusterer.h
   )

ROOT_GENERATE_DICTIONARY(G__RMA ${HEADERS} LINKDEF include/LinkDef.h)
add_library(RadMapAna SHARED ${SRCS} G__RMA)
#set_target_properties(RadMapAna
#                      PROPERTIES
#                      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(RadMapAna ${ROOT_LIBRARIES}) 

# Create main programs using the library

set(EXE_SRCS
    macros/runDummy.C
    macros/crappyTest.C
    macros/convertToCTSEvents.C
    macros/calibrateCTSEvents.C
    macros/readCTSEventTree.C
    macros/tryAroundTracker.C
    )

set(EXE_NAMES
    runDummy
    runCrappyTest
    convertToCTSEvents
    calibrateCTSEvents
    readCTSEventTree
    tryAroundTracker
    )

list(LENGTH EXE_SRCS count)
math(EXPR count "${count}-1")
foreach(i RANGE ${count})
  list(GET EXE_SRCS ${i} src)
  list(GET EXE_NAMES ${i} name)
  add_executable(${name} ${src})
  set_target_properties(${name}
                      PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
  target_link_libraries(${name} RadMapAna)
endforeach()
