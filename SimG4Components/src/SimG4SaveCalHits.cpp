#include "SimG4SaveCalHits.h"

// FCCSW
#include "SimG4Common/Geant4CaloHit.h"
#include "SimG4Interface/IGeoSvc.h"
#include "SimG4Common/Units.h"

// Geant4
#include "G4Event.hh"

// datamodel
#include "edm4hep/SimCalorimeterHitCollection.h"

// DD4hep
#include "DDG4/Geant4Hits.h"

DECLARE_COMPONENT(SimG4SaveCalHits)

SimG4SaveCalHits::SimG4SaveCalHits(const std::string& aType, const std::string& aName, const IInterface* aParent)
    : GaudiTool(aType, aName, aParent), m_geoSvc("GeoSvc", aName) {
  declareInterface<ISimG4SaveOutputTool>(this);
  declareProperty("CaloHits", m_caloHits, "Handle for calo hits");
  declareProperty("GeoSvc", m_geoSvc);
}

SimG4SaveCalHits::~SimG4SaveCalHits() {}

StatusCode SimG4SaveCalHits::initialize() {
  if (GaudiTool::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  if (!m_geoSvc) {
    error() << "Unable to locate Geometry Service. "
            << "Make sure you have GeoSvc and SimSvc in the right order in the configuration." << endmsg;
    return StatusCode::FAILURE;
  }
  auto lcdd = m_geoSvc->lcdd();
  auto allReadouts = lcdd->readouts();
  for (auto& readoutName : m_readoutNames) {
    if (allReadouts.find(readoutName) == allReadouts.end()) {
      error() << "Readout " << readoutName << " not found! Please check tool configuration." << endmsg;
      return StatusCode::FAILURE;
    } else {
      debug() << "Hits will be saved to EDM from the collection " << readoutName << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode SimG4SaveCalHits::finalize() { return GaudiTool::finalize(); }

StatusCode SimG4SaveCalHits::saveOutput(const G4Event& aEvent) {
  G4HCofThisEvent* collections = aEvent.GetHCofThisEvent();
  G4VHitsCollection* collect;
  k4::Geant4CaloHit* hit;
  if (collections != nullptr) {
    auto edmHits = m_caloHits.createAndPut();
    for (int iter_coll = 0; iter_coll < collections->GetNumberOfCollections(); iter_coll++) {
      collect = collections->GetHC(iter_coll);
      if (std::find(m_readoutNames.begin(), m_readoutNames.end(), collect->GetName()) != m_readoutNames.end()) {
        size_t n_hit = collect->GetSize();
        debug() << "\t" << n_hit << " hits are stored in a collection #" << iter_coll << ": " << collect->GetName()
                << endmsg;
        for (size_t iter_hit = 0; iter_hit < n_hit; iter_hit++) {
          hit = dynamic_cast<k4::Geant4CaloHit*>(collect->GetHit(iter_hit));
          edm4hep::SimCalorimeterHit edmHit = edmHits->create();
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
