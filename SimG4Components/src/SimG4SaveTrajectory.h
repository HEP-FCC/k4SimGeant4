#ifndef SIMG4COMPONENTS_G4SAVETRAJECTORY
#define SIMG4COMPONENTS_G4SAVETRAJECTORY

// Gaudi
#include "GaudiAlg/GaudiTool.h"

// FCCSW
#include "k4FWCore/DataHandle.h"
#include "SimG4Interface/ISimG4SaveOutputTool.h"
class IGeoSvc;

#if __has_include("edm4hep/TrackerHit3DCollection.h")
#include "edm4hep/TrackerHit3DCollection.h"
#else
#include "edm4hep/TrackerHitCollection.h"
namespace edm4hep {
  using TrackerHit3DCollection = edm4hep::TrackerHitCollection;
}
#endif

/** @class SimG4SaveTrajectory SimG4Components/src/SimG4SaveTrajectory.h SimG4SaveTrajectory.h
 *
 * Tool to save Geant4 Trajectory data. Requires Geant to be run with the command "/tracking/storeTrajectory 1".
 *  Note that access to trajectories is expensive, so this tool should only be used for debugging and visualisation.
 *
 */

class SimG4SaveTrajectory : public GaudiTool, virtual public ISimG4SaveOutputTool {
public:
  explicit SimG4SaveTrajectory(const std::string& aType, const std::string& aName, const IInterface* aParent);
  virtual ~SimG4SaveTrajectory();
  /**  Initialize.
   *   @return status code
   */
  virtual StatusCode initialize();
  /**  Finalize.
   *   @return status code
   */
  virtual StatusCode finalize();
  /**  Save the data output.
   *   @param[in] aEvent Event with data to save.
   *   @return status code
   */
  virtual StatusCode saveOutput(const G4Event& aEvent) final;

private:
  /// Pointer to the geometry service
  ServiceHandle<IGeoSvc> m_geoSvc;
  /// Handle for trajectory hits including position information
  DataHandle<edm4hep::TrackerHit3DCollection> m_trackHits{"Hits/Trajectory",
                                                                      Gaudi::DataHandle::Writer, this};
};

#endif /* SIMG4COMPONENTS_G4SAVETRAJECTORY */
