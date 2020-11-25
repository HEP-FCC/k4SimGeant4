################################################################################
# Package: SimG4Full
################################################################################

file(GLOB _lib_sources src/lib/*.cpp)
gaudi_add_library(SimG4Full
                 SOURCES ${_lib_sources}
                 LINK GaudiAlgLib k4FWCore::k4FWCore SimG4Common EDM4HEP::edm4hep ${Geant4_LIBRARIES} SimG4Interface)

#target_include_directories(SimG4Full PUBLIC ${Geant4_INCLUDE_DIRS}
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
#  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

file(GLOB _module_sources src/components/*.cpp)
gaudi_add_module(SimG4FullPlugins
                 SOURCES ${_module_sources}
                 LINK GaudiAlgLib k4FWCore::k4FWCore SimG4Common EDM4HEP::edm4hep DD4hep::DDCore SimG4Interface SimG4Full)
