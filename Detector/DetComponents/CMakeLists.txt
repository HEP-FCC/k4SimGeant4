################################################################################
#Package : DetComponents
################################################################################

#--- Add main module
file(GLOB _sources src/*.cpp)
gaudi_add_module(DetComponents
                 SOURCES ${_sources}
                 LINK
                      DD4hep::DDCore
                      DD4hep::DDG4
                      DD4hep::DDRec
                      k4FWCore::k4FWCore
                      k4FWCore::k4Interface
                      Gaudi::GaudiKernel
                      EDM4HEP::edm4hep
                      ROOT::Core
                      ROOT::Hist
)

install(TARGETS DetComponents
        EXPORT ${PROJECT_NAME}Targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
        COMPONENT dev
)

add_test(NAME MagFieldScanner
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
         COMMAND bash -c "source k4simgeant4env.sh; k4run ${CMAKE_CURRENT_LIST_DIR}/tests/options/magFieldScanner.py"
)

#
#include(CTest)
#gaudi_add_test(RedoSegmentationXYZ
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/redoSegmentationXYZ.py)
#gaudi_add_test(RedoSegmentationPhiEta
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/redoSegmentationPhiEta.py)
#gaudi_add_test(MergeCells
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/mergeCells.py)
#gaudi_add_test(MergeLayers
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/mergeLayers.py)
#gaudi_add_test(positionsInclinedEcal
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/positions_inclinedEcal.py)
#gaudi_add_test(positionsInclinedEcalCheck
#               ENVIRONMENT PYTHONPATH+=${PODIO_PYTHON_DIR}
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               COMMAND python Detector/DetComponents/tests/scripts/check_inclinedEcal_positions.py
#               DEPENDS positionsInclinedEcal)
#gaudi_add_test(positionsTracker
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/positions_tracker.py)
#gaudi_add_test(positionsPositiveCaloEndcap
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/positions_endcap.py)
#gaudi_add_test(positionsPositiveCaloEndcapCheck
#               ENVIRONMENT PYTHONPATH+=${PODIO_PYTHON_DIR}
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               COMMAND python Detector/DetComponents/tests/scripts/check_endcapEcal_sublayer_positions.py
#               DEPENDS positionsPositiveCaloEndcap)
#gaudi_add_test(positionsNegativeCaloEndcap
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/positions_negativeEndcap.py)
#gaudi_add_test(positionsNegativeCaloEndcapCheck
#               ENVIRONMENT PYTHONPATH+=${PODIO_PYTHON_DIR}
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               COMMAND python Detector/DetComponents/tests/scripts/check_negativeEndcapEcal_sublayer_positions.py
#               DEPENDS positionsNegativeCaloEndcap)
#gaudi_add_test(RewriteBitfield
#               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#               FRAMEWORK tests/options/rewriteBitfield.py)
