#include "SimG4SaveCalHits.h"

// FCCSW
#include "SimG4Common/Geant4CaloHit.h"
#include "SimG4Interface/IGeoSvc.h"
#include "SimG4Common/Units.h"

// Geant4
#include "G4Event.hh"

// datamodel
#include "edm4hep/CaloHitContributionCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"

// DD4hep
#include "DD4hep/Detector.h"
#include "DD4hep/Segmentations.h"


DECLARE_COMPONENT(SimG4SaveCalHits)

SimG4SaveCalHits::SimG4SaveCalHits(const std::string& aType, const std::string& aName, const IInterface* aParent)
    : GaudiTool(aType, aName, aParent), m_geoSvc("GeoSvc", aName), m_eventDataSvc("EventDataSvc", "SimG4SaveCalHits")  {
  declareInterface<ISimG4SaveOutputTool>(this);
  declareProperty("CaloHitContributions", m_caloHitContribs, "Handle for calo hit contributions");
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

  StatusCode sc = m_eventDataSvc.retrieve();
  m_podioDataSvc = dynamic_cast<PodioLegacyDataSvc*>(m_eventDataSvc.get());
  if (sc == StatusCode::FAILURE) {
    error() << "Error retrieving Event Data Service" << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode SimG4SaveCalHits::finalize() { return GaudiTool::finalize(); }

StatusCode SimG4SaveCalHits::saveOutput(const G4Event& aEvent) {
  G4HCofThisEvent* eventCollections = aEvent.GetHCofThisEvent();
  if (!eventCollections) {
    debug() << "Event collections not found." << endmsg;
    return StatusCode::SUCCESS;
  }

  auto edmHitContribs = m_caloHitContribs.createAndPut();
  auto edmHits = m_caloHits.createAndPut();
  for (int iter_coll = 0; iter_coll < eventCollections->GetNumberOfCollections(); iter_coll++) {
    G4VHitsCollection* collection = eventCollections->GetHC(iter_coll);
    if (std::find(m_readoutNames.begin(), m_readoutNames.end(), collection->GetName()) == m_readoutNames.end()) {
      continue;
    }

    // Add CellID encoding string to collection metadata
    auto lcdd = m_geoSvc->lcdd();
    auto allReadouts = lcdd->readouts();
    auto idspec = lcdd->idSpecification(collection->GetName());
    auto field_str = idspec.fieldDescription();
    auto& coll_md = m_podioDataSvc->getProvider().getCollectionMetaData(m_caloHits.get()->getID());
    coll_md.setValue("CellIDEncodingString", field_str);

    // Lump hit contributions together
    size_t nHit = collection->GetSize();
    std::map<uint64_t, std::vector<k4::Geant4CaloHit*>> contribsInCells;
    for (size_t i = 0; i < nHit; ++i) {
      auto hit = dynamic_cast<k4::Geant4CaloHit*>(collection->GetHit(i));
      contribsInCells[hit->cellID].emplace_back(hit);
    }

    for (const auto& [cellID, contribVec] : contribsInCells) {
      // Cell energy
      double energyDeposit = 0;
      for (const auto* contrib : contribVec) {
        energyDeposit += contrib->energyDeposit;
      }

      // Cell position
      dd4hep::DDSegmentation::CellID volumeID = cellID;
      auto detElement = lcdd->volumeManager().lookupDetElement(volumeID);
      auto transformMatrix = detElement.nominal().worldTransformation();
      auto segmentation = lcdd->readout(collection->GetName()).segmentation().segmentation();
      const dd4hep::DDSegmentation::Vector3D cellPositionVecLocal = segmentation->position(cellID);

      double cellPositionLocal[] = {cellPositionVecLocal.x(),
                                    cellPositionVecLocal.y(),
                                    cellPositionVecLocal.z()};
      double cellPositionGlobal[3];
      transformMatrix.LocalToMaster(cellPositionLocal, cellPositionGlobal);

      // Fill the cell hit and contributions
      auto edmHit = edmHits->create();
      edmHit.setCellID(cellID);
      edmHit.setEnergy(energyDeposit * sim::g42edm::energy);
      edmHit.setPosition({
        (float) cellPositionGlobal[0] * (float) sim::d4h2edm::length,
        (float) cellPositionGlobal[1] * (float) sim::d4h2edm::length,
        (float) cellPositionGlobal[2] * (float) sim::d4h2edm::length,
      });

      debug() << "Cell ID: " << edmHit.getCellID() << endmsg;
      debug() << "Cell energy: " << edmHit.getEnergy() << endmsg;
      debug() << "Cell global position:" << endmsg;
      debug() << "  x: " << edmHit.getPosition().x << endmsg;
      debug() << "  y: " << edmHit.getPosition().y << endmsg;
      debug() << "  z: " << edmHit.getPosition().z << endmsg;

      for (const auto* contrib : contribVec) {
        auto edmHitContrib = edmHitContribs->create();
        edmHitContrib.setPDG(contrib->pdgId);
        edmHitContrib.setEnergy(contrib->energyDeposit * sim::g42edm::energy);
        edmHitContrib.setTime(contrib->time);
        edmHitContrib.setStepPosition({
            (float) contrib->position.x() * (float) sim::g42edm::length,
            (float) contrib->position.y() * (float) sim::g42edm::length,
            (float) contrib->position.z() * (float) sim::g42edm::length,
        });
        edmHit.addToContributions(edmHitContrib);

        debug() << "Contribution:" << endmsg;
        debug() << "  PDG ID: " << edmHitContrib.getPDG() << endmsg;
        debug() << "  energy:  " << edmHitContrib.getEnergy() << endmsg;
        debug() << "  time:  " << edmHitContrib.getTime() << endmsg;
        debug() << "  position: " << endmsg;
        debug() << "    x: " << edmHitContrib.getStepPosition().x << endmsg;
        debug() << "    y: " << edmHitContrib.getStepPosition().y << endmsg;
        debug() << "    z: " << edmHitContrib.getStepPosition().z << endmsg;
        debug() << "  track ID: " << contrib->trackId << endmsg;
      }

    }
  }

  return StatusCode::SUCCESS;
}
