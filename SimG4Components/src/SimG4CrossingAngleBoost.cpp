#include "SimG4CrossingAngleBoost.h"

// FCCSW
#include "SimG4Common/ParticleInformation.h"
#include "SimG4Common/Units.h"

// Geant4
#include "G4Event.hh"

// datamodel
#include "edm4hep/MCParticleCollection.h"

// DD4hep
#include "DD4hep/Segmentations.h"

DECLARE_COMPONENT(SimG4CrossingAngleBoost)

SimG4CrossingAngleBoost::SimG4CrossingAngleBoost(const std::string& aName,ISvcLocator* aSvcLoc) : GaudiAlgorithm(aName, aSvcLoc) {
  declareProperty("InParticles", m_inParticles, "Handle for the input particles");
  declareProperty("OutParticles", m_outParticles, "Handle for the output particles");
}

StatusCode SimG4CrossingAngleBoost::initialize() {

  StatusCode sc = GaudiAlgorithm::initialize();
  return sc;
}

StatusCode SimG4CrossingAngleBoost::execute() {

  auto outParticles = m_outParticles.createAndPut();
  auto inParticles = m_inParticles.get();

  debug() << "Input particle collection size: " << inParticles->size() << endmsg;

  for (auto const& inParticle: *inParticles) {
    auto outParticle = outParticles->create();
    outParticle.setPDG(inParticle.getPDG());
    outParticle.setGeneratorStatus(inParticle.getGeneratorStatus());
    outParticle.setSimulatorStatus(inParticle.getSimulatorStatus());
    outParticle.setCharge(inParticle.getCharge());
    outParticle.setTime(inParticle.getTime());
    outParticle.setMass(inParticle.getMass());
    outParticle.setVertex(inParticle.getVertex());
    outParticle.setEndpoint(inParticle.getEndpoint());
    outParticle.setMomentum(inParticle.getMomentum());
    outParticle.setMomentumAtEndpoint(inParticle.getMomentumAtEndpoint());
    outParticle.setSpin(inParticle.getSpin());
    outParticle.setColorFlow(inParticle.getColorFlow());
  }

  debug() << "Output particle collection size: " << outParticles->size() << endmsg;

  return StatusCode::SUCCESS;
}

StatusCode SimG4CrossingAngleBoost::finalize() { return GaudiAlgorithm::finalize(); }
