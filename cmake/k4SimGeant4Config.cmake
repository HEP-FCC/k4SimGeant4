
include(CMakeFindDependencyMacro)
find_dependency(Geant4 REQUIRED)
find_dependency(k4FWCore REQUIRED)

# - Include the targets file to create the imported targets that a client can
# link to (libraries) or execute (programs)
include("${CMAKE_CURRENT_LIST_DIR}/k4SimGeant4Targets.cmake")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(k4SimGeant4  DEFAULT_MSG CMAKE_CURRENT_LIST_FILE)
