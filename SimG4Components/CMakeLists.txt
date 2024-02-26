################################################################################
# Package: SimG4Components
################################################################################


file(GLOB _lib_sources src/*.cpp)
gaudi_add_module(SimG4Components
                 SOURCES ${_lib_sources}
                 LINK
                      CLHEP::CLHEP
                      DD4hep::DDCore
                      DD4hep::DDG4
                      ${Geant4_LIBRARIES}
                      SimG4Common
                      SimG4Interface
                      k4FWCore::k4FWCore
                      k4FWCore::k4Interface
                      EDM4HEP::edm4hep
                      Gaudi::GaudiAlgLib
)

add_test(NAME CrossingAngleBoost
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
         COMMAND bash -c "source k4simgeant4env.sh; k4run ${CMAKE_CURRENT_LIST_DIR}/tests/options/xAngleBoost.py"
)

add_test(NAME MagFieldFromMap
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
         COMMAND bash -c "source k4simgeant4env.sh;  k4run ${CMAKE_CURRENT_LIST_DIR}/tests/options/magFieldFromMap.py"
)

add_test(NAME MagFieldFromDD4hep
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
         COMMAND bash -c "source k4simgeant4env.sh;  k4run ${CMAKE_CURRENT_LIST_DIR}/tests/options/magFieldTool.py"
)
add_test(NAME OpticalPhysicsTest
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
         COMMAND bash -c "source k4simgeant4env.sh;  k4run ${CMAKE_CURRENT_LIST_DIR}/tests/options/optical_physics_test.py"
)
SET_TESTS_PROPERTIES( OpticalPhysicsTest PROPERTIES FAIL_REGULAR_EXPRESSION " Exception;EXCEPTION;ERROR;Error" )
SET_TESTS_PROPERTIES( OpticalPhysicsTest PROPERTIES PASS_REGULAR_EXPRESSION " Cerenkov process active:               1;  Scintillation process active:          1;  Rayleigh process active:               1;  Absorption process active:             1")
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
