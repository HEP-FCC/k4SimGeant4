#ifndef SIMG4COMPONENTS_G4SIMALG_H
#define SIMG4COMPONENTS_G4SIMALG_H

// GAUDI
#include "Gaudi/Algorithm.h"

// FCCSW
#include "k4FWCore/DataHandle.h"
#include "SimG4Interface/ISimG4EventProviderTool.h"
#include "SimG4Interface/ISimG4SaveOutputTool.h"

// Forward declarations:
// Interfaces
class ISimG4Svc;

// Geant
class G4Event;

/** @class SimG4Alg SimG4Components/src/SimG4Alg.h SimG4Alg.h
 *
 *  Geant simulation algorithm.
 *  Controls the event flow: translates the EDM event to G4Event, passes it to SimG4Svc,
 *  retrieves it after the finished simulation, and stores the output as specified in tools.
 *  It takes MCParticleCollection (\b'genParticles') as the input
 *  as well as a list of names of tools that define the EDM output (\b'outputs').
 *  [For more information please see](@ref md_sim_doc_geant4fullsim).
 *
 *  @author Anna Zaborowska
 */

class SimG4Alg : public Gaudi::Algorithm {
public:
  explicit SimG4Alg(const std::string&, ISvcLocator*);
  virtual ~SimG4Alg();
  /**  Initialize.
   *   @return status code
   */
  virtual StatusCode initialize() final;
  /**  Execute the simulation.
   *   Translation of MCParticleCollection to G4Event is done using EDM2G4() method.
   *   Then, G4Event is passed to SimG4Svc for the simulation and retrieved afterwards.
   *   The tools m_saveTools are used to save the output from the retrieved events.
   *   Finally, the event is terminated.
   *   @return status code
   */
  virtual StatusCode execute(const EventContext&) const final;
  /**  Finalize.
   *   @return status code
   */
  virtual StatusCode finalize() final;

private:
  /// Pointer to the interface of Geant simulation service
  ServiceHandle<ISimG4Svc> m_geantSvc;
  /// Handle to the tools saving the output
  mutable PublicToolHandleArray<ISimG4SaveOutputTool> m_saveTools {
      this, "outputs", {}};
  /// Handle for the tool that creates the G4Event
  mutable ToolHandle<ISimG4EventProviderTool> m_eventTool{
      "SimG4PrimariesFromEdmTool", this};
};
#endif /* SIMG4COMPONENTS_G4SIMALG_H */
