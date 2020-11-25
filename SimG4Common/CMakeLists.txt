################################################################################
# Package: SimG4Common
################################################################################


file(GLOB _lib_sources src/*.cpp)
gaudi_add_library(SimG4Common
                 SOURCES ${_lib_sources}
                 INCLUDE_DIRS FCCFWCore Geant4 ROOT FCCEDM PODIO
                 LINK GaudiAlgLib k4FWCore::k4FWCore {Geant4_LIBRARIES} EDM4HEP::edm4hep
                 PUBLIC_HEADERS SimG4Common)
target_include_directories(SimG4Common PUBLIC ${Geant4_INCLUDE_DIRS}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
