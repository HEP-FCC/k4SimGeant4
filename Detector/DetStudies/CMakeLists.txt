################################################################################
#Package : DetStudies
################################################################################


find_package(ROOT COMPONENTS MathCore GenVector Geom Tree)

#--- Add main module
file(GLOB _sources src/components/*.cpp)
gaudi_add_module(DetStudies
                 SOURCES ${_sources}
                 LINK
                      DD4hep::DDCore
                      DD4hep::DDG4
                      k4FWCore::k4FWCore
                      k4FWCore::k4Interface
                      Gaudi::GaudiAlgLib
                      Gaudi::GaudiKernel
                      EDM4HEP::edm4hep
                      ROOT::Core
                      ROOT::Hist
                )

install(TARGETS DetStudies
  EXPORT FCCDetectorTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)


install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/tests DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/DetStudies)


##TODO:
#include(CTest)
#gaudi_add_test(samplingFractionInclinedEcal
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/samplingFraction_inclinedEcal.py)
#
#gaudi_add_test(upstreamMaterialInclinedEcal
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/upstreamMaterial_inclinedEcal.py)
