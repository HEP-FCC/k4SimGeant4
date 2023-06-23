#include "MagFieldProbe.h"

// Geant4
#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"

// ROOT
#include "TH2D.h"
#include "TFile.h"

struct XYPlaneProbe {
  double xMax;
  double yMax;
  double z;
};

struct ZPlaneProbe {
  double zMin;
  double zMax;
  double rMax;
  double phi;
};

struct CylinderProbe {
  double r;
  double zMin;
  double zMax;
};

inline bool parseXYPlaneProbe(const std::vector<std::string>& probeDef,
                              XYPlaneProbe& probe) {
  if (probeDef.size() != 4) {
    return true;
  }
  probe.xMax = std::stod(probeDef.at(1));
  probe.yMax = std::stod(probeDef.at(2));
  probe.z = std::stod(probeDef.at(3));

  return false;
}

inline bool parseZPlaneProbe(const std::vector<std::string>& probeDef,
                             ZPlaneProbe& probe) {
  if (probeDef.size() != 5) {
    return true;
  }
  probe.zMin = std::stod(probeDef.at(1));
  probe.zMax = std::stod(probeDef.at(2));
  probe.rMax = std::stod(probeDef.at(3));
  probe.phi = std::stod(probeDef.at(4));

  return false;
}

inline bool parseCylinderProbe(const std::vector<std::string>& probeDef,
                               CylinderProbe& probe) {
  if (probeDef.size() != 4) {
    return true;
  }
  probe.r = std::stod(probeDef.at(1));
  probe.zMin = std::stod(probeDef.at(2));
  probe.zMax = std::stod(probeDef.at(3));

  return false;
}

std::ostream& operator<<(std::ostream& outStream, const XYPlaneProbe& probe) {
  return outStream << "xyPlane: xMax = " << probe.xMax
                   << ", yMax = " << probe.yMax
                   << ", z = " << probe.z;
}

std::ostream& operator<<(std::ostream& outStream, const ZPlaneProbe& probe) {
  return outStream << "zPlane: zMin = " << probe.zMin
                   << ", zMax = " << probe.zMax
                   << ", rMax = " << probe.rMax
                   << ", phi = " << probe.phi;
}

std::ostream& operator<<(std::ostream& outStream, const CylinderProbe& probe) {
  return outStream << "cylinder: r = " << probe.r
                   << ", zMin = " << probe.zMin
                   << ", zMax = " << probe.zMax;
}


MagFieldProbe::MagFieldProbe(const std::string& name,
                             ISvcLocator* svcLoc) : Service(name, svcLoc),
                                                    m_geoSvc("GeoSvc", name),
                                                    m_simG4Svc("SimG4Svc", name) {}



StatusCode MagFieldProbe::initialize() {
  {
    StatusCode sc = Service::initialize();
    if (sc.isFailure()) {
      return sc;
    }
  }

  if (!m_geoSvc) {
    error() << "Unable to find Geometry Service!" << endmsg;
    return StatusCode::FAILURE;
  }

  if (!m_simG4Svc) {
    error() << "Unable to find Geant4 Service!" << endmsg;
    return StatusCode::FAILURE;
  }

  const G4TransportationManager* transpManager =
      G4TransportationManager::GetTransportationManager();
  if (!transpManager) {
    error() << "Unable to find Geant4 Transportation Manager!" << endmsg;
    return StatusCode::FAILURE;
  }

  const G4FieldManager* fieldManager = transpManager->GetFieldManager();
  if (!fieldManager->DoesFieldExist()) {
    error() << "No Geant4 field found!" << endmsg;
  }

  const G4MagneticField* magField =
      dynamic_cast<const G4MagneticField*>(fieldManager->GetDetectorField());
  if (!magField) {
    error() << "Found Geant4 field is not a magnetic field!" << endmsg;
  }

  debug() << "Probe results will be written to:" << endmsg;
  debug() << "  " << m_outFilePath.value() << endmsg;

  std::vector<XYPlaneProbe> xyPlaneProbes;
  std::vector<ZPlaneProbe> zPlaneProbes;
  std::vector<CylinderProbe> cylinderProbes;
  for (const auto& probeDef : m_probes.value()) {
    if (probeDef.size() == 0) {
      continue;
    }

    if (probeDef.at(0) == "xyPlane") {
      XYPlaneProbe xyPlaneProbe;
      bool err = parseXYPlaneProbe(probeDef, xyPlaneProbe);
      if (!err) {
        xyPlaneProbes.emplace_back(xyPlaneProbe);
      }
    } else if (probeDef.at(0) == "zPlane") {
      ZPlaneProbe zPlaneProbe;
      bool err = parseZPlaneProbe(probeDef, zPlaneProbe);
      if (!err) {
        zPlaneProbes.emplace_back(zPlaneProbe);
      }
    } else if (probeDef.at(0) == "Cylinder") {
      CylinderProbe cylinderProbe;
      bool err = parseCylinderProbe(probeDef, cylinderProbe);
      if (!err) {
        cylinderProbes.emplace_back(cylinderProbe);
      }
    } else {
      warning() << "Probe of type '" << probeDef.at(0) << "' not recognized!"
                << endmsg;
    }
  }

  if (xyPlaneProbes.size()) {
    info() << "Defined xyPlane probes:" << endmsg;
    for (const auto& probe : xyPlaneProbes) {
      info() << probe << endmsg;
    }
  }
  if (xyPlaneProbes.size()) {
    info() << "Defined zPlane probes:" << endmsg;
    for (const auto& probe : zPlaneProbes) {
      info() << probe << endmsg;
    }
  }
  if (xyPlaneProbes.size()) {
    info() << "Defined cylinder probes:" << endmsg;
    for (const auto& probe : cylinderProbes) {
      info() << probe << endmsg;
    }
  }

  std::vector<TH2D> xyPlaneProbeHistosX;
  std::vector<TH2D> xyPlaneProbeHistosY;
  std::vector<TH2D> xyPlaneProbeHistosZ;
  for (const auto& probe : xyPlaneProbes) {
    std::string histName = "xyPlane_";
    histName += std::to_string((int) probe.xMax) + "_";
    histName += std::to_string((int) probe.yMax) + "_";
    histName += std::to_string((int) probe.z) + "_bField";
    std::string histTitle = "xyPlane, z = ";
    histTitle += std::to_string((int) probe.z) + " mm, bField";

    auto histX = TH2D((histName + "_x").c_str(),
                      (histTitle + "(x)").c_str(),
                      500, -probe.xMax, probe.xMax,
                      500, -probe.yMax, probe.yMax);
    histX.GetXaxis()->SetTitle("x [mm]");
    histX.GetYaxis()->SetTitle("y [mm]");
    histX.GetZaxis()->SetTitle("B_{x} [T]");

    auto histY = TH2D((histName + "_y").c_str(),
                      (histTitle + "(y)").c_str(),
                      500, -probe.xMax, probe.xMax,
                      500, -probe.yMax, probe.yMax);
    histY.GetXaxis()->SetTitle("x [mm]");
    histY.GetYaxis()->SetTitle("y [mm]");
    histY.GetZaxis()->SetTitle("B_{y} [T]");


    auto histZ = TH2D((histName + "_z").c_str(),
                      (histTitle + "(z)").c_str(),
                      500, -probe.xMax, probe.xMax,
                      500, -probe.yMax, probe.yMax);
    histZ.GetXaxis()->SetTitle("x [mm]");
    histZ.GetYaxis()->SetTitle("y [mm]");
    histZ.GetZaxis()->SetTitle("B_{z} [T]");

    xyPlaneProbeHistosX.emplace_back(histX);
    xyPlaneProbeHistosY.emplace_back(histY);
    xyPlaneProbeHistosZ.emplace_back(histZ);
  }

  for (size_t iProbe = 0; iProbe < xyPlaneProbes.size(); ++iProbe) {
    auto& histX = xyPlaneProbeHistosX.at(iProbe);
    for (int i = 1; i <= histX.GetXaxis()->GetNbins(); ++i) {
      for (int j = 1; j <= histX.GetYaxis()->GetNbins(); ++j) {
        double point[] = {0., 0., 0., 0.};
        point[0] = histX.GetXaxis()->GetBinCenter(i);
        point[1] = histX.GetYaxis()->GetBinCenter(j);
        point[2] = xyPlaneProbes.at(iProbe).z;
        double field[] = {0., 0., 0.};
        magField->GetFieldValue(point, field);
        histX.SetBinContent(i, j, field[0]);
        xyPlaneProbeHistosY.at(iProbe).SetBinContent(i, j, field[1]);
        xyPlaneProbeHistosZ.at(iProbe).SetBinContent(i, j, field[2]);
      }
    }
  }

  auto outFile = TFile(m_outFilePath.value().c_str(), "RECREATE");
  for (const auto& hist : xyPlaneProbeHistosX) {
    hist.Write();
  }
  for (const auto& hist : xyPlaneProbeHistosY) {
    hist.Write();
  }
  for (const auto& hist : xyPlaneProbeHistosZ) {
    hist.Write();
  }
  outFile.Write();
  outFile.Close();

  return StatusCode::SUCCESS;
}


StatusCode MagFieldProbe::finalize() { return StatusCode::SUCCESS; }


DECLARE_COMPONENT(MagFieldProbe)
