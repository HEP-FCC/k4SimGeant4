#include "k4Interface/IGeoSvc.h"

#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/Service.h"

/** @class MaterialScan_2D Detector/DetComponents/src/MaterialScan_2D.h MaterialScan_2D.h
 *
 *  Service that facilitates material scan on initialize
 *  This service outputs a ROOT file containing a TTree with radiation lengths and material thickness
 *  For an example on how to read the file, see Examples/scripts/material_plots.py
 *
 *  @author J. Lingemann
 */

class MaterialScan_2D : public Service {
public:
  explicit MaterialScan_2D(const std::string& name, ISvcLocator* svcLoc);

  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual ~MaterialScan_2D(){};

private:
  /// name of the output file
  Gaudi::Property<std::string> m_filename{this, "filename", "", "file name to save the tree to"};
  /// Handle to the geometry service from which the detector is retrieved
  ServiceHandle<IGeoSvc> m_geoSvc;


  /// Step size in eta/theta/cosTheta/thetaRad
  Gaudi::Property<double> m_angleBinning{this, "angleBinning", 0.05, "eta/theta/cosTheta/thetaRad bin size"};
  /// Maximum eta/theta/cosTheta/thetaRad until which to scan
  Gaudi::Property<double> m_angleMax{this, "angleMax", 6, "maximum eta/theta/cosTheta/thetaRad value"};
  /// Minimum eta/theta/cosTheta/thetaRad until which to scan
  Gaudi::Property<double> m_angleMin{this, "angleMin", -6, "minimum eta/theta/cosTheta/thetaRad value"};
  /// number of phi values to run over, evenly distributed from 0 to 2 pi
  Gaudi::Property<double> m_nPhi{this, "nPhi", 100,
                                       "number of phi values to run over, evenly distributed from 0 to 2 pi"};
  /// angle definition to use: eta, theta, cosTheta or thetaRad default: eta
  Gaudi::Property<std::string> m_angleDef{this, "angleDef", "eta",
                                       "angle definition to use: 'eta', 'theta' (in degrees), 'cosTheta' or 'thetaRad' (in rad), default: 'eta'"};
  /// Name of the envelope within which the material is measured (by default: world volume)
  Gaudi::Property<std::string> m_envelopeName{this, "envelopeName", "world",
                                              "name of the envelope within which the material is measured"};
  /// Flat random number generator
  Rndm::Numbers m_flatAngleDist;

};
