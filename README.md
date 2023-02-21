# k4SimGeant4

Gaudi Components for Geant4 Simulation in the Key4HEP software framework.
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

* Gaudi (=>v35r0)
* k4FWCore (=>1.0)
* Geant4
* DD4hep
* EDM4HEP
