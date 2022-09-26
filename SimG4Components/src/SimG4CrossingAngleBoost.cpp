#include "SimG4CrossingAngleBoost.h"

// k4SimGeant4
#include "SimG4Common/ParticleInformation.h"
#include "SimG4Common/Units.h"

// Geant4
#include "G4Event.hh"

// CLHEP
#include "CLHEP/Units/PhysicalConstants.h"

// EDM4HEP
#include "edm4hep/MCParticleCollection.h"

// DD4hep
#include "DD4hep/Segmentations.h"


DECLARE_COMPONENT(SimG4CrossingAngleBoost)


SimG4CrossingAngleBoost::SimG4CrossingAngleBoost(
    const std::string& aName,
    ISvcLocator* aSvcLoc) : GaudiAlgorithm(aName, aSvcLoc) {
  declareProperty("InParticles", m_inParticles,
                  "Handle for the input particles");
  declareProperty("OutParticles", m_outParticles,
                  "Handle for the output particles");
}


StatusCode SimG4CrossingAngleBoost::initialize() {
  StatusCode sc;

  if (m_alpha != 0.) {
    debug() << "Boosting particle according to crossing angle alpha = "
            << m_alpha << "rad" << endmsg;
    sc = GaudiAlgorithm::initialize();
  } else {
    error() << "Crossing angle alpha = " << m_alpha.value() << "." << endmsg;
    error() << "There is no need to run this Algorithm." << endmsg;
    sc = StatusCode::FAILURE;
  }

  return sc;
}


StatusCode SimG4CrossingAngleBoost::execute() {

  auto outParticles = m_outParticles.createAndPut();
  auto inParticles = m_inParticles.get();

  debug() << "Input particle collection size: " << inParticles->size()
          << endmsg;

  double alpha = m_alpha;
  double gamma = std::sqrt(1 + std::pow(std::tan(alpha), 2));
  double betagamma = std::tan(alpha);

  for (auto const& inParticle: *inParticles) {
    auto outParticle = inParticle.clone();

    debug() << "---------------------------------------------------" << endmsg;
    debug() << "Particle:" << endmsg;
    debug() << "  - PDG ID: " << outParticle.getPDG() << endmsg;
    debug() << "  - time: " << outParticle.getTime() << endmsg;
    debug() << "  - vertex: " << endmsg;
    debug() << "    - x: " << outParticle.getVertex().x << endmsg;
    debug() << "    - y: " << outParticle.getVertex().y << endmsg;
    debug() << "    - z: " << outParticle.getVertex().z << endmsg;
    debug() << "  - momentum: " << endmsg;
    debug() << "    - px: " << outParticle.getMomentum().x << endmsg;
    debug() << "    - py: " << outParticle.getMomentum().y << endmsg;
    debug() << "    - pz: " << outParticle.getMomentum().z << endmsg;

    double t = gamma * outParticle.getTime() +
               betagamma * outParticle.getVertex().x / CLHEP::c_light;
    double x = gamma * outParticle.getVertex().x +
               betagamma * CLHEP::c_light * outParticle.getTime();
    double y = outParticle.getVertex().y;
    double z = outParticle.getVertex().z;

    double e2 = pow(outParticle.getMomentum().x, 2) +
                pow(outParticle.getMomentum().y, 2) +
                pow(outParticle.getMomentum().z, 2) +
                pow(outParticle.getMass(), 2);
    float px = betagamma * std::sqrt(e2) + gamma * outParticle.getMomentum().x;
    float py = outParticle.getMomentum().y;
    float pz = outParticle.getMomentum().z;

    outParticle.setTime(t);
    outParticle.setVertex({x, y, z});
    outParticle.setMomentum({px, py, pz});

    debug() << "" << endmsg;
    debug() << "~~~ BOOST ~~~" << endmsg;
    debug() << "" << endmsg;

    debug() << "Particle:" << endmsg;
    debug() << "  - PDG ID: " << outParticle.getPDG() << endmsg;
    debug() << "  - time: " << outParticle.getTime() << endmsg;
    debug() << "  - vertex: " << endmsg;
    debug() << "    - x: " << outParticle.getVertex().x << endmsg;
    debug() << "    - y: " << outParticle.getVertex().y << endmsg;
    debug() << "    - z: " << outParticle.getVertex().z << endmsg;
    debug() << "  - momentum: " << endmsg;
    debug() << "    - px: " << outParticle.getMomentum().x << endmsg;
    debug() << "    - py: " << outParticle.getMomentum().y << endmsg;
    debug() << "    - pz: " << outParticle.getMomentum().z << endmsg;

    outParticles->push_back(outParticle);
  }

  debug() << "Output particle collection size: " << outParticles->size()
          << endmsg;

  return StatusCode::SUCCESS;
}


StatusCode SimG4CrossingAngleBoost::finalize() {
  return GaudiAlgorithm::finalize();
}
