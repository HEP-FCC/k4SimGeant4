#ifndef SIMG4COMPONENTS_CROSSINGANGLEBOOST_H
#define SIMG4COMPONENTS_CROSSINGANGLEBOOST_H

// Gaudi
#include "Gaudi/Algorithm.h"

// FCCSW
#include "k4FWCore/DataHandle.h"

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

class SimG4CrossingAngleBoost : public Gaudi::Algorithm {
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
  StatusCode execute(const EventContext&) const;

 private:
  /// Handle for the particles to be read
  mutable DataHandle<edm4hep::MCParticleCollection> m_inParticles{"InParticles", Gaudi::DataHandle::Reader, this};
  /// Handle for the particles to be written
  mutable DataHandle<edm4hep::MCParticleCollection> m_outParticles{"OutParticles", Gaudi::DataHandle::Writer, this};
  /// Value of the crossing angle in radians
  Gaudi::Property<double> m_alpha{this, "CrossingAngle", 0., "Crossing angle (alpha) in radians"};
};

#endif /* SIMG4COMPONENTS_CROSSINGANGLEBOOST_H */
