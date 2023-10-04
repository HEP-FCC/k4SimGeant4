#include "SimG4SaveTrackerHits.h"

// k4SimGeant4
#include "SimG4Common/Units.h"
#include "SimG4Common/Geant4PreDigiTrackHit.h"

// Geant4
#include "G4Event.hh"

// DD4hep
#include "DD4hep/Detector.h"


DECLARE_COMPONENT(SimG4SaveTrackerHits)

SimG4SaveTrackerHits::SimG4SaveTrackerHits(const std::string& aType,
                                           const std::string& aName,
                                           const IInterface* aParent)
    : GaudiTool(aType, aName, aParent),
      m_geoSvc("GeoSvc", aName) {
  declareInterface<ISimG4SaveOutputTool>(this);
  declareProperty("SimTrackHits", m_trackHits, "Handle for tracker hits");
  declareProperty("GeoSvc", m_geoSvc);
}

SimG4SaveTrackerHits::~SimG4SaveTrackerHits() {}

StatusCode SimG4SaveTrackerHits::initialize() {
  if (GaudiTool::initialize().isFailure()) {
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
    warning() << "Please, use \"readoutName\" property instead." << endmsg;

    if (m_readoutNames.size() > 1) {
      error() << "More than one readout name provided. Exiting..." << endmsg;
      return StatusCode::FAILURE;
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
           << m_trackHits.objKey() << "\"." << endmsg;
  }

  // Add CellID encoding string to hit collection metadata
  auto idspec = lcdd->idSpecification(m_readoutName);
  auto field_str = idspec.fieldDescription();
  m_cellIDEncoding.put(field_str);
  debug() << "Storing cell ID encoding string: \"" << field_str << "\"."
          << endmsg;

  return StatusCode::SUCCESS;
}

StatusCode SimG4SaveTrackerHits::finalize() { return GaudiTool::finalize(); }

StatusCode SimG4SaveTrackerHits::saveOutput(const G4Event& aEvent) {
  G4HCofThisEvent* collections = aEvent.GetHCofThisEvent();
  G4VHitsCollection* collect;
  k4::Geant4PreDigiTrackHit* hit;
  if (collections != nullptr) {
    edm4hep::SimTrackerHitCollection* edmHits = m_trackHits.createAndPut();
    for (int iter_coll = 0; iter_coll < collections->GetNumberOfCollections(); iter_coll++) {
      collect = collections->GetHC(iter_coll);
      if (m_readoutName == collect->GetName()) {
        size_t n_hit = collect->GetSize();
        verbose() << "\t" << n_hit << " hits are stored in a tracker collection #" << iter_coll << ": "
               << collect->GetName() << endmsg;
        for (size_t iter_hit = 0; iter_hit < n_hit; iter_hit++) {
          hit = dynamic_cast<k4::Geant4PreDigiTrackHit*>(collect->GetHit(iter_hit));
          auto edmHit = edmHits->create();
          edmHit.setCellID(hit->cellID);
          edmHit.setEDep(hit->energyDeposit * sim::g42edm::energy);
          /// workaround, store trackid in an unrelated field
          edmHit.setQuality(hit->trackId);
          edmHit.setTime(hit->time);
          edmHit.setPosition({
                              hit->prePos.x() * sim::g42edm::length,
                              hit->prePos.y() * sim::g42edm::length,
                              hit->prePos.z() * sim::g42edm::length,
          });
          CLHEP::Hep3Vector diff = hit->postPos - hit->prePos;
          edmHit.setMomentum({
                               (float) (diff.x() * sim::g42edm::length),
                               (float) (diff.y() * sim::g42edm::length),
                               (float) (diff.z() * sim::g42edm::length),
          });
          edmHit.setPathLength(diff.mag());
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}
