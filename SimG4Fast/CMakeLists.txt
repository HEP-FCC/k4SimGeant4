################################################################################
# Package: SimG4Fast
################################################################################


file(GLOB _lib_sources src/lib/*.cpp)
gaudi_add_library(SimG4Fast
                 SOURCES ${_lib_sources}
                 LINK GaudiAlgLib k4FWCore::k4FWCore SimG4Common EDM4HEP::edm4hep DD4hep::DDCore SimG4Interface)


file(GLOB _module_sources src/components/*.cpp)
gaudi_add_module(SimG4FastPlugins
                 SOURCES ${_module_sources}
                 LINK GaudiAlgLib k4FWCore::k4FWCore SimG4Common EDM4HEP::edm4hep DD4hep::DDCore SimG4Interface SimG4Fast)

