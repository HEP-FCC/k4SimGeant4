################################################################################
# Package: SimG4Fast
################################################################################


file(GLOB _lib_sources src/lib/*.cpp)
gaudi_add_library(SimG4Fast
                 SOURCES ${_lib_sources}
                 LINK DD4hep::DDCore SimG4Common SimG4Interface k4FWCore::k4FWCore EDM4HEP::edm4hep)


file(GLOB _module_sources src/components/*.cpp)
gaudi_add_module(SimG4FastPlugins
                 SOURCES ${_module_sources}
                 LINK DD4hep::DDCore SimG4Common SimG4Interface SimG4Fast k4FWCore::k4FWCore EDM4HEP::edm4hep)

