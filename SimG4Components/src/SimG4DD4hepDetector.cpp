#include "SimG4DD4hepDetector.h"

// k4FWCore
#include "k4Interface/IGeoSvc.h"

// Geant4
#include "G4VUserDetectorConstruction.hh"

DECLARE_COMPONENT(SimG4DD4hepDetector)

SimG4DD4hepDetector::SimG4DD4hepDetector(const std::string& aType, const std::string& aName, const IInterface* aParent)
    : AlgTool(aType, aName, aParent), m_geoSvc("GeoSvc", aName) {
  declareInterface<ISimG4DetectorConstruction>(this);
  declareProperty("GeoSvc", m_geoSvc);
}

SimG4DD4hepDetector::~SimG4DD4hepDetector() {}

StatusCode SimG4DD4hepDetector::initialize() {
  if (AlgTool::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  if (!m_geoSvc) {
    error() << "Unable to locate Geometry Service. "
            << "Make sure you have GeoSvc and SimSvc in the right order in the configuration." << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode SimG4DD4hepDetector::finalize() { return AlgTool::finalize(); }

G4VUserDetectorConstruction* SimG4DD4hepDetector::detectorConstruction() { return m_geoSvc->getGeant4Geo(); }
