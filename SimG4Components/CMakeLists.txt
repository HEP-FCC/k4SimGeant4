################################################################################
# Package: SimG4Components
################################################################################



file(GLOB _lib_sources src/*.cpp)
gaudi_add_module(SimG4Components
                 SOURCES ${_lib_sources}
                 LINK Gaudi::GaudiAlgLib k4FWCore::k4FWCore SimG4Common EDM4HEP::edm4hep DD4hep::DDCore SimG4Interface)


#include(CTest)
#gaudi_add_test(GeantFullSimGdml
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/geant_fullsim_gdml.py)
#gaudi_add_test(GeantFullSimHCal
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/geant_fullsim_hcal.py)
#gaudi_add_test(GeantFullSimMoreEvents
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Sim/SimG4Components/tests/
#               FRAMEWORK tests/options/geant_fullsim_moreEvents.py)
#gaudi_add_test(GeantFullSimMoreEventsCheckNumEvents
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Sim/SimG4Components/tests/
#               COMMAND python ./scripts/geant_fullsim_moreEvents_checkNumEvents.py
#               DEPENDS GeantFullSimMoreEvents)
#gaudi_add_test(GeantFullSimMoreEventsCheckNumParticles
#               ENVIRONMENT PYTHONPATH+=${PODIO_PYTHON_DIR}
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Sim/SimG4Components/tests/
#               COMMAND python ./scripts/geant_fullsim_moreEvents_checkNumParticles.py
#               DEPENDS GeantFullSimMoreEvents)
#gaudi_add_test(GeantFastSimSimpleSmearing
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Sim/SimG4Components/tests/
#               FRAMEWORK tests/options/geant_fastsim_simple.py)
#add_test(      NAME  SimG4Components.GeantFastSimSimpleSmearingCheckSigma
#               COMMAND python ./scripts/geant_fastsim_simple_checkSigma.py
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Sim/SimG4Components/tests/ )
#set_tests_properties(SimG4Components.GeantFastSimSimpleSmearingCheckSigma PROPERTIES DEPENDS SimG4Components.GeantFastSimSimpleSmearing)
#gaudi_add_test(GeantFastSimSimpleSmearingCheckNumParticles
#               ENVIRONMENT PYTHONPATH+=${PODIO_PYTHON_DIR}
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Sim/SimG4Components/tests/
#               COMMAND python ./scripts/geant_fastsim_checkNumParticles.py
#               DEPENDS GeantFastSimSimpleSmearing)
#
