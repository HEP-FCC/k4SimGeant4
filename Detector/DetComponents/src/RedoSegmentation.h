#ifndef DETCOMPONENTS_REDOSEGMENTATION_H
#define DETCOMPONENTS_REDOSEGMENTATION_H

// GAUDI
#include "Gaudi/Algorithm.h"
#include "GaudiKernel/ToolHandle.h"

// k4FWCore
#include "k4FWCore/DataHandle.h"
#include "k4FWCore/MetaDataHandle.h"
#include "k4Interface/IGeoSvc.h"

// DD4hep
#include "DD4hep/Readout.h"
#include "DD4hep/Segmentations.h"

// EDM4hep
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/Constants.h"

/** @class RedoSegmentation Detector/DetComponents/src/RedoSegmentation.h RedoSegmentation.h
 *
 *  Redo the segmentation after the simulation has ended.
 *  True positions of the hits are required!
 *  New readout (with new segmentation) has to be added to <readouts> tag in the detector description xml.
 *  Cell IDs are rewritten from the old readout (`\b oldReadoutName`) to the new readout (`\b newReadoutName`).
 *  Names of the old segmentation fields need to be passed as a vector '\b oldSegmentationIds'.
 *  Those fields are replaced by the new segmentation.
 *
 *  For an example see Detector/DetComponents/tests/options/redoSegmentationXYZ.py
 *  and Detector/DetComponents/tests/options/redoSegmentationRPhi.py.
 *
 *  @author Anna Zaborowska
 */

class RedoSegmentation : public Gaudi::Algorithm {
public:
  explicit RedoSegmentation(const std::string&, ISvcLocator*);
  virtual ~RedoSegmentation();
  /**  Initialize.
   *   @return status code
   */
  virtual StatusCode initialize() final;
  /**  Execute.
   *   @return status code
   */
  virtual StatusCode execute(const EventContext&) const final;
  /**  Finalize.
   *   @return status code
   */
  virtual StatusCode finalize() final;

private:
  /**  Get ID of the volume that contains the cell.
   *   @param[in] aCellId ID of the cell.
   *   @return ID of the volume.
   */
  uint64_t volumeID(uint64_t aCellId) const;
  /// Pointer to the geometry service
  ServiceHandle<IGeoSvc> m_geoSvc;
  /// Handle for the EDM positioned hits to be read
  mutable DataHandle<edm4hep::CalorimeterHitCollection> m_inHits{
      "hits/caloInHits", Gaudi::DataHandle::Reader, this};
  /// Handle for the EDM hits to be written
  mutable DataHandle<edm4hep::SimCalorimeterHitCollection> m_outHits{
      "hits/caloOutHits", Gaudi::DataHandle::Writer, this};
  /// Handle for the output hits cell id encoding.
  MetaDataHandle<std::string> m_outHitsCellIDEncoding{
      m_outHits, edm4hep::labels::CellIDEncoding, Gaudi::DataHandle::Writer};
  /// New segmentation
  dd4hep::DDSegmentation::Segmentation* m_segmentation;
  int m_segmentationType; // use enum instead? defined in some namespace?
  /// Name of the detector readout used in simulation
  Gaudi::Property<std::string> m_oldReadoutName{this, "oldReadoutName", "",
                                                "Name of the detector readout used in simulation"};
  /// Old segmentation
  dd4hep::DDSegmentation::Segmentation* m_oldSegmentation;
  int m_oldSegmentationType; // use enum instead? defined in some namespace?
  /// Name of the new detector readout
  Gaudi::Property<std::string> m_newReadoutName{this, "newReadoutName", "", "Name of the new detector readout"};
  /// Old bitfield decoder
  dd4hep::DDSegmentation::BitFieldCoder* m_oldDecoder;
  /// Segmentation fields that are going to be replaced by the new segmentation
  Gaudi::Property<std::vector<std::string>> m_oldIdentifiers{
      this, "oldSegmentationIds", {}, "Segmentation fields that are going to be replaced by the new segmentation"};
  /// Detector fields that are going to be rewritten
  std::vector<std::string> m_detectorIdentifiers;
  /// Limit of debug printing
  Gaudi::Property<uint> m_debugPrint{this, "debugPrint", 10, "Limit of debug printing"};
};
#endif /* DETCOMPONENTS_REDOSEGMENTATION_H */
