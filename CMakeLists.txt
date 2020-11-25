CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

project(k4SimGeant4)

find_package(ROOT COMPONENTS RIO Tree)

#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects
find_package(Gaudi)
find_package(k4FWCore)
find_package(EDM4HEP)
find_package(Geant4)
find_package(DD4hep)
#---------------------------------------------------------------

include(GNUInstallDirs)
include(CTest)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/InstallArea/ CACHE PATH
    "Install path prefix, prepended onto install directories." FORCE )
endif()

# Set up C++ Standard
# ``-DCMAKE_CXX_STANDARD=<standard>`` when invoking CMake
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

if(NOT CMAKE_CXX_STANDARD MATCHES "14|17")
  message(FATAL_ERROR "Unsupported C++ standard: ${CMAKE_CXX_STANDARD}")
endif()

message(${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
message(${CMAKE_MODULE_PATH})

add_subdirectory(SimG4Interface)
add_subdirectory(SimG4Common)
add_subdirectory(SimG4Fast)
add_subdirectory(SimG4Components)
add_subdirectory(SimG4Full)






install(EXPORT ${PROJECT_NAME}Targets
  NAMESPACE ${PROJECT_NAME}::
  FILE "${PROJECT_NAME}Targets.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/"
  )

gaudi_install(CMAKE cmake/${PROJECT_NAME}Config.cmake)

