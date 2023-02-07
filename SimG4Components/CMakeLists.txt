################################################################################
# Package: SimG4Components
################################################################################



file(GLOB _lib_sources src/*.cpp)
gaudi_add_module(SimG4Components
                 SOURCES ${_lib_sources}
                 LINK Gaudi::GaudiAlgLib
                      k4FWCore::k4FWCore
                      SimG4Common
                      EDM4HEP::edm4hep
                      DD4hep::DDCore
                      DD4hep::DDG4
                      CLHEP::CLHEP
                      SimG4Interface)


include(CTest)

set(GAUDI_GENCONF_DIR "genConfDir")
if (${Gaudi_VERSION} VERSION_LESS 35.1)
  set(GAUDI_GENCONF_DIR "genConf")
endif()

function(set_test_env _testname)
  set_property(TEST ${_testname} APPEND PROPERTY ENVIRONMENT
    LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}:$<TARGET_FILE_DIR:SimG4Components>:$<TARGET_FILE_DIR:ROOT::Core>:$<TARGET_FILE_DIR:k4FWCore::k4FWCore>:$<TARGET_FILE_DIR:EDM4HEP::edm4hep>:$<TARGET_FILE_DIR:podio::podio>:$ENV{LD_LIBRARY_PATH}
    PYTHONPATH=$<TARGET_FILE_DIR:SimG4Components>/${GAUDI_GENCONF_DIR}:$<TARGET_FILE_DIR:k4FWCore::k4FWCore>/../python:$<TARGET_FILE_DIR:SimG4Components>/../python:$ENV{PYTHONPATH}
    PATH=$<TARGET_FILE_DIR:k4FWCore::k4FWCore>/../bin:$ENV{PATH}
    K4SIMGEANT4=${CMAKE_CURRENT_LIST_DIR}/
    )
endfunction()

add_test(NAME CrossingAngleBoost
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         COMMAND k4run ${CMAKE_CURRENT_LIST_DIR}/tests/options/xAngleBoost.py
)
set_test_env(CrossingAngleBoost)

add_test(NAME MagFieldFromMap
         WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
         COMMAND k4run ${CMAKE_CURRENT_LIST_DIR}/tests/options/magFieldFromMap.py
)
set_test_env(MagFieldFromMap)

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
