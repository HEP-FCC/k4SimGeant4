#include "SimG4SaveCalHits.h"

// k4SimGeant4
#include "SimG4Common/Geant4CaloHit.h"
#include "SimG4Common/Units.h"

// Geant4
#include "G4Event.hh"

// DD4hep
#include "DD4hep/Detector.h"
#include "DD4hep/Segmentations.h"


DECLARE_COMPONENT(SimG4SaveCalHits)

SimG4SaveCalHits::SimG4SaveCalHits(const std::string& aType,
                                   const std::string& aName,
                                   const IInterface* aParent) :
      AlgTool(aType, aName, aParent), m_geoSvc("GeoSvc", aName) {
  declareInterface<ISimG4SaveOutputTool>(this);
  declareProperty("CaloHits", m_caloHits, "Handle for calo hits");
  declareProperty("GeoSvc", m_geoSvc);
}

SimG4SaveCalHits::~SimG4SaveCalHits() {}

StatusCode SimG4SaveCalHits::initialize() {
  if (AlgTool::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }

  if (!m_geoSvc) {
    error() << "Unable to locate Geometry Service. "
            << "Make sure you have GeoSvc and SimSvc in the right order in the configuration." << endmsg;
    return StatusCode::FAILURE;
  }

  if (m_readoutName.empty() && m_readoutNames.empty()) {
    error() << "No readout name provided. Exiting..." << endmsg;
    return StatusCode::FAILURE;
  }

  if (!m_readoutName.empty() && !m_readoutNames.empty()) {
    error() << "Readout name provided through \"readoutName\" parameter, "
            << "but also through deprecated \"readoutNames\" vector "
            << "parameter." << endmsg;
    error() << "Please use only the \"readoutName\" parameter. Exiting..."
            << endmsg;
    return StatusCode::FAILURE;
  }

  if (!m_readoutNames.empty()) {
    warning() << "Providing multiple readout names deprecated." << endmsg;
    warning() << "Please use \"readoutName\" parameter instead." << endmsg;

    if (m_readoutNames.size() > 1) {
      error() << "More than one readout name provided. Exiting..." << endmsg;
    }

    m_readoutName = m_readoutNames[0];
  }

  if (m_readoutName.empty()) {
    error() << "No readout name provided. Exiting..." << endmsg;
    return StatusCode::FAILURE;
  }

  auto lcdd = m_geoSvc->getDetector();
  auto allReadouts = lcdd->readouts();
  if (allReadouts.find(m_readoutName) == allReadouts.end()) {
    error() << "Readout " << m_readoutName << " not found! "
            << "Please check tool configuration.  Exiting..." << endmsg;
      return StatusCode::FAILURE;
  } else {
    info() << "Hits from readout \"" << m_readoutName.value()
           << "\" will be saved in the collection \""
           << m_caloHits.objKey() << "\"." << endmsg;
  }

  // Add CellID encoding string to hit collection metadata
  auto idspec = lcdd->idSpecification(m_readoutName);
  auto field_str = idspec.fieldDescription();
  m_cellIDEncoding.put(field_str);
  debug() << "Storing cell ID encoding string: \"" << field_str << "\"."
          << endmsg;

  return StatusCode::SUCCESS;
}

StatusCode SimG4SaveCalHits::finalize() { return AlgTool::finalize(); }

StatusCode SimG4SaveCalHits::saveOutput(const G4Event& aEvent) {
  G4HCofThisEvent* collections = aEvent.GetHCofThisEvent();
  G4VHitsCollection* collect;
  k4::Geant4CaloHit* hit;
  if (collections != nullptr) {
    auto edmHits = m_caloHits.createAndPut();
    for (int iter_coll = 0; iter_coll < collections->GetNumberOfCollections(); iter_coll++) {
      collect = collections->GetHC(iter_coll);
      if (m_readoutName == collect->GetName()) {
        size_t n_hit = collect->GetSize();
        debug() << "\t" << n_hit << " hits are stored in a collection #" << iter_coll << ": " << collect->GetName()
                << endmsg;
        for (size_t iter_hit = 0; iter_hit < n_hit; iter_hit++) {
          hit = dynamic_cast<k4::Geant4CaloHit*>(collect->GetHit(iter_hit));
          auto edmHit = edmHits->create();
          edmHit.setCellID(hit->cellID);
          //todo
          //edmHitCore.bits = hit->trackId;
          edmHit.setEnergy(hit->energyDeposit * sim::g42edm::energy);
          edmHit.setPosition({
                       (float) hit->position.x() * (float) sim::g42edm::length,
                       (float) hit->position.y() * (float) sim::g42edm::length,
                       (float) hit->position.z() * (float) sim::g42edm::length,
          });
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}
