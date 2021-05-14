################################################################################
# Package: SimG4Common
################################################################################


file(GLOB _lib_sources src/*.cpp)
gaudi_add_library(SimG4Common
                 SOURCES ${_lib_sources}
                 LINK Gaudi::GaudiAlgLib k4FWCore::k4FWCore ${Geant4_LIBRARIES} EDM4HEP::edm4hep
                 )
#target_include_directories(SimG4Common PUBLIC ${Geant4_INCLUDE_DIRS}
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
#  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS SimG4Common
  EXPORT k4SimGeant4Targets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
