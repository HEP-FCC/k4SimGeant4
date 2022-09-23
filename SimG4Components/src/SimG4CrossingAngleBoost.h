#ifndef SIMG4COMPONENTS_CROSSINGANGLEBOOST_H
#define SIMG4COMPONENTS_CROSSINGANGLEBOOST_H

// Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

// FCCSW
#include "k4FWCore/DataHandle.h"
#include "SimG4Interface/ISimG4SaveOutputTool.h"
#include "SimG4Interface/ISimG4ParticleSmearTool.h"

// datamodel
namespace edm4hep {
class MCParticleCollection;
}

/** @class SimG4CrossingAngleBoost SimG4Components/src/SimG4CrossingAngleBoost.h SimG4CrossingAngleBoost.h
 *
 *  Boost 'generated' particles according the crossing angle.
 *
 *  @author Juraj Smiesko, Gerri Ganis
 */

class SimG4CrossingAngleBoost : public GaudiAlgorithm {
 public:
  SimG4CrossingAngleBoost(const std::string& aName, ISvcLocator* svcLoc);
  /**  Initialize.
   *   @return status code
   */
  StatusCode initialize();
  /**  Finalize.
   *   @return status code
   */
  StatusCode finalize();
  /**  Save the data output.
   *   Saves the 'reconstructed' (smeared) particles and associates them with MC particles.
   *   @param[in] aEvent Event with data to save.
   *   @return status code
   */
  StatusCode execute();

 private:
  /// Handle for the particles to be read
  DataHandle<edm4hep::MCParticleCollection> m_inParticles{"InParticles", Gaudi::DataHandle::Reader, this};
  /// Handle for the particles to be written
  DataHandle<edm4hep::MCParticleCollection> m_outParticles{"OutParticles", Gaudi::DataHandle::Writer, this};
};

#endif /* SIMG4COMPONENTS_CROSSINGANGLEBOOST_H */
