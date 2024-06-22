#ifndef SIMG4COMPONENTS_G4SAVETRACKERHITS_H
#define SIMG4COMPONENTS_G4SAVETRACKERHITS_H

// STL
#include <vector>
#include <string>

// Gaudi
#include "GaudiAlg/GaudiTool.h"

// k4FWCore
#include "k4FWCore/DataHandle.h"
#include "k4FWCore/MetaDataHandle.h"
#include "k4Interface/IGeoSvc.h"
#include "k4Interface/ISimG4SaveOutputTool.h"

// EDM4hep
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/Constants.h"

/** @class SimG4SaveTrackerHits SimG4Components/src/SimG4SaveTrackerHits.h SimG4SaveTrackerHits.h
 *
 *  \brief Save tracker hits tool.
 *
 *  The tool expects one readout name and will produce one collection.
 *
 *  Readout name is defined in DD4hep compact file as the attribute `readout` of
 *  a `detector` tag.
 *
 *  If readout name which does not correspond to any Geant4 hit collection is
 *  provided, the tool will fail at initialization.
 *
 *  If both `readoutName` and deprecated `readoutNames` are provided, the tool
 *  will fail at initialization.
 *
 *  If the more than one readout names is provided through the deprecated
 *  `readoutNames` parameter, the tool will fail at initialization.
 *
 *  [For more information please see](@ref md_sim_doc_geant4fullsim).
 *
 *  @author Anna Zaborowska
 *  @author Valentin Volkl (extended with Digi Info)
 *  @author Juraj Smiesko (deprecated `readoutNames`)
 */

class SimG4SaveTrackerHits : public GaudiTool, virtual public ISimG4SaveOutputTool {
public:
  explicit SimG4SaveTrackerHits(const std::string& aType, const std::string& aName, const IInterface* aParent);
  virtual ~SimG4SaveTrackerHits();
  /**  Initialize.
   *   @return status code
   */
  virtual StatusCode initialize();
  /**  Finalize.
   *   @return status code
   */
  virtual StatusCode finalize();
  /**  Save the data output.
   *   Saves the tracker hits from the collections as specified in the job options in \b'readoutNames'.
   *   @param[in] aEvent Event with data to save.
   *   @return status code
   */
  virtual StatusCode saveOutput(const G4Event& aEvent) final;

private:
  /// Pointer to the geometry service
  ServiceHandle<IGeoSvc> m_geoSvc;
  /// Handle for output tracker hits
  DataHandle<edm4hep::SimTrackerHitCollection> m_trackHits {
      "TrackerHits", Gaudi::DataHandle::Writer, this};
  /// Output handle for cell ID encoding string
  MetaDataHandle<std::string> m_cellIDEncoding {
      m_trackHits, edm4hep::labels::CellIDEncoding, Gaudi::DataHandle::Writer};
  /// Names of the readouts (hits collections) to save, deprecated
  Gaudi::Property<std::vector<std::string>> m_readoutNames {
      this, "readoutNames", {}, "[Deprecated] Name of the readouts (hits collections) to save"};
  /// Name of the readout (hits collection) to save
  Gaudi::Property<std::string> m_readoutName {
      this, "readoutName", {}, "Name of the readout (hit collection) to save"};
};

#endif /* SIMG4COMPONENTS_G4SAVETRACKERHITS_H */
