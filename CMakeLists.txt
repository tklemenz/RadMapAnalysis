cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(RadMapAnalysis)

set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
set(CMAKE_MODULE_PATH $ENV{ROOTSYS}/etc/cmake)
find_package(ROOT REQUIRED COMPONENTS MathCore RIO Hist Tree Net)

set(CMAKE_BUILD_TYPE_SHARED_LIBS ON)

#find_package(fmt REQUIRED)

#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#find_package(Boost 1.65.0 COMPONENTS log)

add_definitions(${ROOT_CXX_FLAGS})

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS})# ${Boost_INCLUDE_DIRS})

set(SRCS
   src/Dummy.cxx
   src/Signal.cxx
   src/Fiber.cxx
   src/EventBase.cxx
   src/CTSEvent.cxx
   src/Module.cxx
   src/Utility.cxx
   src/Constants.cxx
   src/Cluster.cxx
   src/CTSEventClusters.cxx
   src/Track.cxx
   src/Tracker.cxx
   src/Clusterer.cxx
   src/ClusterEvent.cxx
   )

set(HEADERS
   include/Dummy.h
   include/Signal.h
   include/Fiber.h
   include/EventBase.h
   include/CTSEvent.h
   include/Module.h
   include/Utility.h
   include/Constants.h
   include/Cluster.h
   include/CTSEventClusters.h
   include/Track.h
   include/Tracker.h
   include/Clusterer.h
   include/ClusterEvent.h
   )

ROOT_GENERATE_DICTIONARY(G__RMA ${HEADERS} LINKDEF include/LinkDef.h)
add_library(RadMapAna SHARED ${SRCS} G__RMA)
target_link_libraries(RadMapAna ${ROOT_LIBRARIES})
                                #fmt::fmt
                                #${Boost_LIBRARIES})

#install(TARGETS RadMapAna ${INSTALL_OPTS})

# Create main programs using the library

set(EXE_SRCS
    macros/runDummy.C
    macros/crappyTest.C
    macros/convertToCTSEvents.C
    macros/calibrateCTSEventsConfig0.C
    macros/readCTSEventTree.C
    macros/tryAroundTracker.C
    macros/plotCTSEvent.C
    macros/convertToCTSEventsCluster.C
    macros/plotClusterEvent.C
    macros/convertToClusterEvents.C
    macros/plotCtsEventCluster.C
    macros/getToTCalibrationValues.C
    macros/convertToCalibratedCTSEvents.C
    macros/getPadiwaGainMap.C
    macros/getEfficiencyFromConfig0.C
    macros/getEfficiencyFromConfig2.C
    macros/applyPadiwaGainCorrectionToSignals.C
    macros/getModuleGain.C
    )

set(EXE_NAMES
    runDummy
    runCrappyTest
    convertToCTSEvents
    calibrateCTSEventsConfig0
    readCTSEventTree
    tryAroundTracker
    plotCTSEvent
    convertToCTSEventsCluster
    plotClusterEvent
    convertToClusterEvents
    plotCtsEventCluster
    getToTCalibrationValuesForPionData
    convertToCalibratedCTSEvents
    getPadiwaGainMap
    getEfficiencyFromConfig0
    getEfficiencyFromConfig2
    applyPadiwaGainCorrectionToSignals
    getModuleGain
    )

list(LENGTH EXE_SRCS count)
math(EXPR count "${count}-1")
foreach(i RANGE ${count})
  list(GET EXE_SRCS ${i} src)
  list(GET EXE_NAMES ${i} name)
  add_executable(${name} ${src})
  target_link_libraries(${name} RadMapAna)
  set_target_properties(${name}
                      PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endforeach()
