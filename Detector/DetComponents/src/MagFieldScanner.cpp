#include "MagFieldScanner.h"

// Geant4
#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4SystemOfUnits.hh"

// ROOT
#include "TH2D.h"
#include "TFile.h"


MagFieldScanner::MagFieldScanner(const std::string& name,
                                 ISvcLocator* svcLoc) : Service(name, svcLoc),
                                                        m_geoSvc("GeoSvc", name),
                                                        m_simG4Svc("SimG4Svc", name) {}


StatusCode MagFieldScanner::initialize() {
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
  for (const auto& probeDef : m_xyPlaneProbes.value()) {
    if (probeDef.size() != 3) {
      continue;
    }
    const XYPlaneProbe xyPlaneProbe{probeDef[0], probeDef[1], probeDef[2]};
    xyPlaneProbes.emplace_back(xyPlaneProbe);
  }

  std::vector<ZPlaneProbe> zPlaneProbes;
  for (const auto& probeDef : m_zPlaneProbes.value()) {
    if (probeDef.size() != 4) {
      continue;
    }
    if (probeDef[2] <= 0.) {
      warning() << "zPlane probe defined with negative or zero rMax!" << endmsg;
      continue;
    }
    if (std::fabs(probeDef[3]) > 2 * CLHEP::pi) {
      warning() << "zPlane probe defined with phi larger than 2*pi!" << endmsg;
    }
    const ZPlaneProbe zPlaneProbe{probeDef[0], probeDef[1], probeDef[2],
                                  probeDef[3]};
    zPlaneProbes.emplace_back(zPlaneProbe);
  }

  std::vector<TubeProbe> tubeProbes;
  for (const auto& probeDef : m_tubeProbes.value()) {
    if (probeDef.size() != 3) {
      continue;
    }
    if (probeDef[2] <= 0.) {
      warning() << "Tube probe defined with negative or zero r!" << endmsg;
      continue;
    }
    const TubeProbe tubeProbe{probeDef[0], probeDef[1], probeDef[2]};
    tubeProbes.emplace_back(tubeProbe);
  }


  if (xyPlaneProbes.size()) {
    info() << "Defined xyPlane probes:" << endmsg;
    for (const auto& probe : xyPlaneProbes) {
      info() << probe << endmsg;
    }
  }

  if (zPlaneProbes.size()) {
    info() << "Defined zPlane probes:" << endmsg;
    for (const auto& probe : zPlaneProbes) {
      info() << probe << endmsg;
    }
  }

  if (tubeProbes.size()) {
    info() << "Defined tube probes:" << endmsg;
    for (const auto& probe : tubeProbes) {
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
        const double point[] = {histX.GetXaxis()->GetBinCenter(i),
                                histX.GetYaxis()->GetBinCenter(j),
                                xyPlaneProbes.at(iProbe).z,
                                0.};
        double field[] = {0., 0., 0.};
        magField->GetFieldValue(point, field);
        histX.SetBinContent(i, j, field[0] / tesla);
        xyPlaneProbeHistosY.at(iProbe).SetBinContent(i, j, field[1] / tesla);
        xyPlaneProbeHistosZ.at(iProbe).SetBinContent(i, j, field[2] / tesla);
      }
    }
  }

  std::vector<TH2D> zPlaneProbeHistosX;
  std::vector<TH2D> zPlaneProbeHistosY;
  std::vector<TH2D> zPlaneProbeHistosZ;
  for (const auto& probe : zPlaneProbes) {
    std::string histName = "zPlane_";
    histName += std::to_string((int) probe.zMin) + "_";
    histName += std::to_string((int) probe.zMax) + "_";
    histName += std::to_string((int) probe.rMax) + "_";
    histName += std::to_string((int) probe.phi) + "_bField";
    std::string histTitle = "zPlane, phi = ";
    histTitle += std::to_string((int) probe.phi) + ", bField";

    auto histX = TH2D((histName + "_x").c_str(),
                      (histTitle + "(x)").c_str(),
                      500, probe.zMin, probe.zMax,
                      500, 0., probe.rMax);
    histX.GetXaxis()->SetTitle("z [mm]");
    histX.GetYaxis()->SetTitle("r [mm]");
    histX.GetZaxis()->SetTitle("B_{x} [T]");

    auto histY = TH2D((histName + "_y").c_str(),
                      (histTitle + "(y)").c_str(),
                      500, probe.zMin, probe.zMax,
                      500, 0., probe.rMax);
    histY.GetXaxis()->SetTitle("z [mm]");
    histY.GetYaxis()->SetTitle("r [mm]");
    histY.GetZaxis()->SetTitle("B_{y} [T]");


    auto histZ = TH2D((histName + "_z").c_str(),
                      (histTitle + "(z)").c_str(),
                      500, probe.zMin, probe.zMax,
                      500, 0., probe.rMax);
    histZ.GetXaxis()->SetTitle("z [mm]");
    histZ.GetYaxis()->SetTitle("r [mm]");
    histZ.GetZaxis()->SetTitle("B_{z} [T]");

    zPlaneProbeHistosX.emplace_back(histX);
    zPlaneProbeHistosY.emplace_back(histY);
    zPlaneProbeHistosZ.emplace_back(histZ);
  }

  for (size_t iProbe = 0; iProbe < zPlaneProbes.size(); ++iProbe) {
    auto& histX = zPlaneProbeHistosX.at(iProbe);
    const auto& probe = zPlaneProbes.at(iProbe);
    for (int i = 1; i <= histX.GetXaxis()->GetNbins(); ++i) {
      for (int j = 1; j <= histX.GetYaxis()->GetNbins(); ++j) {
        const double z = histX.GetXaxis()->GetBinCenter(i);
        const double r = histX.GetYaxis()->GetBinCenter(j);
        const double point[] = {r * std::cos(probe.phi),
                                r * std::sin(probe.phi),
                                z,
                                0};
        double field[] = {0., 0., 0.};
        magField->GetFieldValue(point, field);
        histX.SetBinContent(i, j, field[0] / tesla);
        zPlaneProbeHistosY.at(iProbe).SetBinContent(i, j, field[1] / tesla);
        zPlaneProbeHistosZ.at(iProbe).SetBinContent(i, j, field[2] / tesla);
      }
    }
  }

  std::vector<TH2D> tubeProbeHistosX;
  std::vector<TH2D> tubeProbeHistosY;
  std::vector<TH2D> tubeProbeHistosZ;
  for (const auto& probe : tubeProbes) {
    std::string histName = "tube_";
    histName += std::to_string((int) probe.zMin) + "_";
    histName += std::to_string((int) probe.zMax) + "_";
    histName += std::to_string((int) probe.r) + "_bField";
    std::string histTitle = "Tube, r = ";
    histTitle += std::to_string((int) probe.r) + " mm, bField";

    auto histX = TH2D((histName + "_x").c_str(),
                      (histTitle + "(x)").c_str(),
                      500, probe.zMin, probe.zMax,
                      500, 0., 2 * CLHEP::pi);
    histX.GetXaxis()->SetTitle("z [mm]");
    histX.GetYaxis()->SetTitle("#phi");
    histX.GetZaxis()->SetTitle("B_{x} [T]");

    auto histY = TH2D((histName + "_y").c_str(),
                      (histTitle + "(y)").c_str(),
                      500, probe.zMin, probe.zMax,
                      500, 0., 2 * CLHEP::pi);
    histY.GetXaxis()->SetTitle("z [mm]");
    histY.GetYaxis()->SetTitle("#phi");
    histY.GetZaxis()->SetTitle("B_{y} [T]");


    auto histZ = TH2D((histName + "_z").c_str(),
                      (histTitle + "(z)").c_str(),
                      500, probe.zMin, probe.zMax,
                      500, 0., 2 * CLHEP::pi);
    histZ.GetXaxis()->SetTitle("z [mm]");
    histZ.GetYaxis()->SetTitle("#phi");
    histZ.GetZaxis()->SetTitle("B_{z} [T]");

    tubeProbeHistosX.emplace_back(histX);
    tubeProbeHistosY.emplace_back(histY);
    tubeProbeHistosZ.emplace_back(histZ);
  }

  for (size_t iProbe = 0; iProbe < tubeProbes.size(); ++iProbe) {
    auto& histX = tubeProbeHistosX.at(iProbe);
    const auto& probe = tubeProbes.at(iProbe);
    for (int i = 1; i <= histX.GetXaxis()->GetNbins(); ++i) {
      for (int j = 1; j <= histX.GetYaxis()->GetNbins(); ++j) {
        const double z = histX.GetXaxis()->GetBinCenter(i);
        const double phi = histX.GetYaxis()->GetBinCenter(j);
        const double point[] = {probe.r * std::cos(phi),
                                probe.r * std::sin(phi),
                                z,
                                0};
        double field[] = {0., 0., 0.};
        magField->GetFieldValue(point, field);
        histX.SetBinContent(i, j, field[0] / tesla);
        tubeProbeHistosY.at(iProbe).SetBinContent(i, j, field[1] / tesla);
        tubeProbeHistosZ.at(iProbe).SetBinContent(i, j, field[2] / tesla);
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
  for (const auto& hist : zPlaneProbeHistosX) {
    hist.Write();
  }
  for (const auto& hist : zPlaneProbeHistosY) {
    hist.Write();
  }
  for (const auto& hist : zPlaneProbeHistosZ) {
    hist.Write();
  }
  for (const auto& hist : tubeProbeHistosX) {
    hist.Write();
  }
  for (const auto& hist : tubeProbeHistosY) {
    hist.Write();
  }
  for (const auto& hist : tubeProbeHistosZ) {
    hist.Write();
  }
  outFile.Write();
  outFile.Close();

  return StatusCode::SUCCESS;
}


StatusCode MagFieldScanner::finalize() { return StatusCode::SUCCESS; }


std::ostream& operator<<(std::ostream& outStream,
                         const MagFieldScanner::XYPlaneProbe& probe) {
  return outStream << "xyPlane: xMax = " << probe.xMax
                   << " mm, yMax = " << probe.yMax
                   << " mm, z = " << probe.z << " mm";
}


std::ostream& operator<<(std::ostream& outStream,
                         const MagFieldScanner::ZPlaneProbe& probe) {
  return outStream << "zPlane: zMin = " << probe.zMin
                   << " mm, zMax = " << probe.zMax
                   << " mm, rMax = " << probe.rMax
                   << " mm, phi = " << probe.phi;
}


std::ostream& operator<<(std::ostream& outStream,
                         const MagFieldScanner::TubeProbe& probe) {
  return outStream << "tube: zMin = " << probe.zMin
                   << " mm, zMax = " << probe.zMax
                   << " mm, r = " << probe.r << " mm";
}


DECLARE_COMPONENT(MagFieldScanner)
