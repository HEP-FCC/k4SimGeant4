#include "RewriteBitfield.h"

// FCCSW
#include "k4Interface/IGeoSvc.h"

// datamodel
#include "edm4hep/CalorimeterHitCollection.h"

// DD4hep
#include "DD4hep/Detector.h"
#include "DDSegmentation/Segmentation.h"

DECLARE_COMPONENT(RewriteBitfield)

RewriteBitfield::RewriteBitfield(const std::string& aName, ISvcLocator* aSvcLoc) : GaudiAlgorithm(aName, aSvcLoc), m_geoSvc("GeoSvc", aName) {
  declareProperty("inhits", m_inHits, "Hit collection with old segmentation (input)");
  declareProperty("outhits", m_outHits, "Hit collection with modified segmentation (output)");
}

RewriteBitfield::~RewriteBitfield() {}

StatusCode RewriteBitfield::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) return StatusCode::FAILURE;
  
  if (!m_geoSvc) {
    error() << "Unable to locate Geometry Service. "
            << "Make sure you have GeoSvc and SimSvc in the right order in the configuration." << endmsg;
    return StatusCode::FAILURE;
  }
  // check if readouts exist
  if (m_geoSvc->lcdd()->readouts().find(m_oldReadoutName) == m_geoSvc->lcdd()->readouts().end()) {
    error() << "Readout <<" << m_oldReadoutName << ">> does not exist." << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_geoSvc->lcdd()->readouts().find(m_newReadoutName) == m_geoSvc->lcdd()->readouts().end()) {
    error() << "Readout <<" << m_newReadoutName << ">> does not exist." << endmsg;
    return StatusCode::FAILURE;
  }
  // Take readout, bitfield from GeoSvc
  m_oldDecoder = m_geoSvc->lcdd()->readout(m_oldReadoutName).idSpec().decoder();
  // segmentation identifiers to be overwritten
  if (m_oldIdentifiers.size() == 0) {
    // it is not an error, maybe no segmentation was used previously
    info() << "No identifiers to remove. Only rewritting the readout" << endmsg;
  }
  // create detector identifiers (= all bitfield ids - segmentation ids)
  for (uint itField = 0; itField < m_oldDecoder->size(); itField++) {
    std::string field = (*m_oldDecoder)[itField].name();
    auto iter = std::find(m_oldIdentifiers.begin(), m_oldIdentifiers.end(), field);
    if (iter == m_oldIdentifiers.end()) {
      m_detectorIdentifiers.push_back(field);
    }
  }
  std::vector<std::string> newFields;
  m_newDecoder = m_geoSvc->lcdd()->readout(m_newReadoutName).idSpec().decoder();
  for (uint itField = 0; itField < m_newDecoder->size(); itField++) {
    newFields.push_back((*m_newDecoder)[itField].name());
  }
  for (const auto& detectorField : m_detectorIdentifiers) {
    auto iter = std::find(newFields.begin(), newFields.end(), detectorField);
    if (iter == newFields.end()) {
      error() << "New readout does not contain field <<" << detectorField << ">> that describes the detector ID."
	      << endmsg;
      return StatusCode::FAILURE;
    }
  }
  info() << "Rewritting the readout bitfield." << endmsg;
  info() << "Old bitfield:\t" << m_oldDecoder->fieldDescription() << endmsg;
  info() << "New bitfield:\t" << m_newDecoder->fieldDescription() << endmsg;

  return StatusCode::SUCCESS;
}

StatusCode RewriteBitfield::execute() {
  const auto inHits = m_inHits.get();
  auto outHits = m_outHits.createAndPut();
  // loop over positioned hits to get the energy deposits: position and cellID
  // cellID contains the volumeID that needs to be copied to the new id
  uint64_t oldid = 0;
  uint debugIter = 0;
  for (const auto& hit : *inHits) {
    auto newHit = outHits->create();
    newHit.setEnergy(hit.getEnergy());
    newHit.setTime(hit.getTime());
    dd4hep::DDSegmentation::CellID cID = hit.getCellID();
    if (debugIter < m_debugPrint) {
      debug() << "OLD: " << m_oldDecoder->valueString(cID) << endmsg;
    }
    // now rewrite all fields except for those to be removed
    dd4hep::DDSegmentation::CellID newID=0;
    for (const auto& detectorField : m_detectorIdentifiers) {
      oldid = m_oldDecoder->get(cID, detectorField);
      m_newDecoder->set(newID, detectorField, oldid);
    }
    newHit.setCellID(newID);
    if (debugIter < m_debugPrint) {
      debug() << "NEW: " << m_newDecoder->valueString(newID) << endmsg;
      debugIter++;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode RewriteBitfield::finalize() { return GaudiAlgorithm::finalize(); }
