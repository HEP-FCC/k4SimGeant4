# k4SimGeant4

[![key4hep-build](https://github.com/HEP-FCC/k4SimGeant4/actions/workflows/key4hep-build.yaml/badge.svg)](https://github.com/HEP-FCC/k4SimGeant4/actions/workflows/key4hep-build.yaml)
[![docs](https://github.com/HEP-FCC/k4SimGeant4/actions/workflows/docs.yml/badge.svg)](https://github.com/HEP-FCC/k4SimGeant4/actions/workflows/docs.yml)

Gaudi Components for Geant4 Simulation in the Key4hep software framework.
See the [FCC Tutorials](https://hep-fcc.github.io/fcc-tutorials/) for
documentation on the usage in FCC.

This is a standard Gaudi/CMake based project:
```sh
source /cvmfs/sw.hsf.org/key4hep/setup.sh

mkdir build install
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install
make install
```

To build also Doxygen generated reference documentation run
```
cd build
make doc

xdg-open doxygen/html/index.html
```

## Dependencies

* Gaudi
* k4FWCore
* Geant4
* DD4hep
* EDM4hep
* ROOT
* CLHEP
