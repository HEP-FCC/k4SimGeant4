#include "RedoSegmentation.h"

// FCCSW
#include "k4Interface/IGeoSvc.h"

// datamodel
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"

// DD4hep
#include "DD4hep/Detector.h"
#include "DDSegmentation/Segmentation.h"

DECLARE_COMPONENT(RedoSegmentation)

RedoSegmentation::RedoSegmentation(const std::string& aName, ISvcLocator* aSvcLoc) : GaudiAlgorithm(aName, aSvcLoc), m_geoSvc("GeoSvc", aName), m_eventDataSvc("EventDataSvc", "RedoSegmentation") {
  declareProperty("inhits", m_inHits, "Hit collection with old segmentation (input)");
  declareProperty("outhits", m_outHits, "Hit collection with modified segmentation (output)");
}

RedoSegmentation::~RedoSegmentation() {}

StatusCode RedoSegmentation::initialize() {
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
  StatusCode sc_dataSvc = m_eventDataSvc.retrieve();
  m_podioDataSvc = dynamic_cast<PodioDataSvc*>(m_eventDataSvc.get());
  if (sc_dataSvc == StatusCode::FAILURE) {
    error() << "Error retrieving Event Data Service" << endmsg;
    return StatusCode::FAILURE;
  }
  // segmentation identifiers to be overwritten
  if (m_oldIdentifiers.size() == 0) {
    // it is not an error, maybe no segmentation was used previously
    warning() << "No previous segmentation identifiers. Volume ID may be recomputed incorrectly." << endmsg;
  }
  // create detector identifiers (= all bitfield ids - segmentation ids)
  for (uint itField = 0; itField < m_oldDecoder->size(); itField++) {
    std::string field = (*m_oldDecoder)[itField].name();
    auto iter = std::find(m_oldIdentifiers.begin(), m_oldIdentifiers.end(), field);
    if (iter == m_oldIdentifiers.end()) {
      m_detectorIdentifiers.push_back(field);
    }
  }
  // Take new segmentation from geometry service
  m_segmentation = m_geoSvc->lcdd()->readout(m_newReadoutName).segmentation().segmentation();
  // check if detector identifiers (old and new) agree
  std::vector<std::string> newFields;
  for (uint itField = 0; itField < m_segmentation->decoder()->size(); itField++) {
    newFields.push_back((*m_segmentation->decoder())[itField].name());
  }
  for (const auto& detectorField : m_detectorIdentifiers) {
    auto iter = std::find(newFields.begin(), newFields.end(), detectorField);
    if (iter == newFields.end()) {
      error() << "New readout does not contain field <<" << detectorField << ">> that describes the detector ID."
              << endmsg;
      return StatusCode::FAILURE;
    }
  }
  info() << "Redoing the segmentation." << endmsg;
  info() << "Old bitfield:\t" << m_oldDecoder->fieldDescription() << endmsg;
  info() << "New bitfield:\t" << m_segmentation->decoder()->fieldDescription() << endmsg;
  info() << "New segmentation is of type:\t" << m_segmentation->type() << endmsg;
  if (m_segmentation->type() == "FCCSWGridModuleThetaMerged")
    m_segmentationType=2;
  else
    m_segmentationType=0;
  m_oldSegmentation = m_geoSvc->lcdd()->readout(m_oldReadoutName).segmentation().segmentation();
  info() << "Old segmentation is of type:\t" << m_oldSegmentation->type() << endmsg;
  if (m_oldSegmentation->type() == "FCCSWGridModuleThetaMerged")
    m_oldSegmentationType=2;
  else
    m_oldSegmentationType=0;
  return StatusCode::SUCCESS;
}

StatusCode RedoSegmentation::execute() {
  const auto inHits = m_inHits.get();
  auto outHits = m_outHits.createAndPut();
  // loop over positioned hits to get the energy deposits: position and cellID
  // cellID contains the volumeID that needs to be copied to the new id
  dd4hep::DDSegmentation::CellID oldid = 0;
  uint debugIter = 0;
  for (const auto& hit : *inHits) {
    auto newHit = outHits->create();
    newHit.setEnergy(hit.getEnergy());
    // SimCalorimeterHit type (needed for createCaloCells which runs after RedoSegmentation) has no time member
    // newHit.setTime(hit.getTime());
    dd4hep::DDSegmentation::CellID cellId = hit.getCellID();
    if (debugIter < m_debugPrint) {
      debug() << "OLD: " << m_oldDecoder->valueString(cellId) << endmsg;
    }
    dd4hep::DDSegmentation::Vector3D position;
    if (m_oldSegmentationType == 2) {
      position = m_oldSegmentation->position(cellId);
    }
    else {
      auto pos = hit.getPosition();
      // factor 10 to convert mm to cm
      position = dd4hep::DDSegmentation::Vector3D (pos.x / 10., pos.y / 10., pos.z / 10.);
    }

    // first calculate proper segmentation fields
    // pass volumeID: we need layer / module information
    // (which is easier/safer to get from cellID than infer from position)
    dd4hep::DDSegmentation::VolumeID vID = volumeID(cellId);
    // for module-theta merged segmentation in which we are replacing
    // initial module number with merged module number, we still want
    // to pass the initial module number to segmentation->cellID(..)
    // as part of the volume ID
    if (m_segmentationType == 2)
      m_segmentation->decoder()->set(vID, "module", m_oldDecoder->get(cellId, "module"));
    dd4hep::DDSegmentation::CellID newCellId = m_segmentation->cellID(position, position, vID);
    // now rewrite all other fields (detector ID)
    for (const auto& detectorField : m_detectorIdentifiers) {
      oldid = m_oldDecoder->get(cellId, detectorField);
      m_segmentation->decoder()->set(newCellId, detectorField, oldid);
    }
    newHit.setCellID(newCellId);
    if (debugIter < m_debugPrint) {
      debug() << "NEW: " << m_segmentation->decoder()->valueString(newCellId) << endmsg;
      debugIter++;
    }
  }
  // Define the metadata
  auto& coll_md = m_podioDataSvc->getProvider().getCollectionMetaData(m_outHits.get()->getID());
  coll_md.setValue("CellIDEncodingString", m_segmentation->decoder()->fieldDescription());
  return StatusCode::SUCCESS;
}

StatusCode RedoSegmentation::finalize() {
  info() << "RedoSegmentation finalize! " << endmsg;
   return GaudiAlgorithm::finalize(); }

uint64_t RedoSegmentation::volumeID(uint64_t aCellId) const {
  dd4hep::DDSegmentation::CellID cID = aCellId;
  for (const auto& identifier : m_oldIdentifiers) {
    m_oldDecoder->set(cID, identifier, 0);
  }
  return cID;
}
