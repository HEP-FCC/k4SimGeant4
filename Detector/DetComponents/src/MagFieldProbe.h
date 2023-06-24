#ifndef MAGFIELDPROBE_H
#define MAGFIELDPROBE_H

// Gaudi
#include "GaudiKernel/Service.h"

// k4FWCore
#include "k4Interface/IGeoSvc.h"
#include "k4Interface/ISimG4Svc.h"
#include "k4Interface/ISimG4MagneticFieldTool.h"


/** @class MagFieldProbe Detector/DetComponents/src/MagFieldProbe.h MagFieldProbe.h
 *
 *  Service probes the Geant4 magnetic field on initialize.
 *  This service outputs a ROOT file containing resulting histograms.
 *
 *  There are three probe types:
 *  * XYPlane probe with parameters: xMax, yMax and z
 *  * ZPlane probe with parameters: zMin, zMax, rMax and phi (angle from x-axis)
 *  * Tube probe with parameters: zMin, zMax and r
 *
 *  @author J. Smiesko
 *  @date 2023-06-23
 */

class MagFieldProbe : public Service {
public:
  explicit MagFieldProbe(const std::string& name, ISvcLocator* svcLoc);

  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual ~MagFieldProbe(){};

private:
  /// Handle to the geometry service
  ServiceHandle<IGeoSvc> m_geoSvc;

  /// Handle to the Geant4 service
  ServiceHandle<ISimG4Svc> m_simG4Svc;

  /// Path to the output file
  Gaudi::Property<std::string> m_outFilePath{this,
                                             "outFilePath",
                                             "magFieldProbe.root",
                                             "Output file path"};

  /// Probes
  Gaudi::Property<std::vector<std::vector<double>>> m_xyPlaneProbes{
      this,
      "xyPlaneProbes",
      {},
      "xy-plane probe definitions"};

  Gaudi::Property<std::vector<std::vector<double>>> m_zPlaneProbes{
      this,
      "zPlaneProbes",
      {},
      "z-plane probe definitions"};

  Gaudi::Property<std::vector<std::vector<double>>> m_tubeProbes{
      this,
      "tubeProbes",
      {},
      "Tube probe definitions"};


  struct XYPlaneProbe {
    const double xMax;
    const double yMax;
    const double z;
  };

  struct ZPlaneProbe {
    const double zMin;
    const double zMax;
    const double rMax;
    const double phi;
  };

  struct TubeProbe {
    const double zMin;
    const double zMax;
    const double r;
  };

  friend std::ostream& operator<<(std::ostream& outStream,
                                  const XYPlaneProbe& probe);
  friend std::ostream& operator<<(std::ostream& outStream,
                                  const ZPlaneProbe& probe);
  friend std::ostream& operator<<(std::ostream& outStream,
                                  const TubeProbe& probe);
};

#endif /* MAGFIELDPROBE_H */
