#include "SimG4CrossingAngleBoost.h"

// CLHEP
#include "CLHEP/Units/PhysicalConstants.h"

// EDM4HEP
#include "edm4hep/MCParticleCollection.h"


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
    debug() << "Boosting particle according to the crossing angle alpha = "
            << m_alpha << "rad" << endmsg;
    sc = GaudiAlgorithm::initialize();
  } else {
    warning() << "Crossing angle alpha = " << m_alpha.value() << " rad." << endmsg;
    warning() << "There is no need to run this algorithm." << endmsg;
  }

  return sc;
}


StatusCode SimG4CrossingAngleBoost::execute() {

  auto outParticles = m_outParticles.createAndPut();
  auto inParticles = m_inParticles.get();

  debug() << "Input particle collection size: " << inParticles->size()
          << endmsg;

  double alpha = -m_alpha;
  double gamma = std::sqrt(1 + std::pow(std::tan(alpha), 2));
  double betagamma = std::tan(alpha);

  for (auto const& inParticle: *inParticles) {
    auto outParticle = inParticle.clone();

    double e = std::sqrt(std::pow(inParticle.getMomentum().x, 2) +
                         std::pow(inParticle.getMomentum().y, 2) +
                         std::pow(inParticle.getMomentum().z, 2) +
                         std::pow(inParticle.getMass(), 2));

    debug() << "---------------------------------------------------" << endmsg;
    debug() << "Particle:" << endmsg;
    debug() << "  - PDG ID: " << inParticle.getPDG() << endmsg;
    debug() << "  - mass: " << inParticle.getMass() << endmsg;
    debug() << "  - vertex: " << endmsg;
    debug() << "    - x: " << inParticle.getVertex().x << endmsg;
    debug() << "    - y: " << inParticle.getVertex().y << endmsg;
    debug() << "    - z: " << inParticle.getVertex().z << endmsg;
    debug() << "  - momentum: " << endmsg;
    debug() << "    - px: " << inParticle.getMomentum().x << endmsg;
    debug() << "    - py: " << inParticle.getMomentum().y << endmsg;
    debug() << "    - pz: " << inParticle.getMomentum().z << endmsg;
    debug() << "  - energy: " << e << endmsg;

    double x = gamma * inParticle.getVertex().x +
               betagamma * CLHEP::c_light * inParticle.getTime();
    double y = inParticle.getVertex().y;
    double z = inParticle.getVertex().z;

    float px = betagamma * e + gamma * inParticle.getMomentum().x;
    float py = inParticle.getMomentum().y;
    float pz = inParticle.getMomentum().z;

    outParticle.setVertex({x, y, z});
    outParticle.setMomentum({px, py, pz});
    double eb = std::sqrt(std::pow(outParticle.getMomentum().x, 2) +
                          std::pow(outParticle.getMomentum().y, 2) +
                          std::pow(outParticle.getMomentum().z, 2) +
                          std::pow(outParticle.getMass(), 2));

    debug() << "" << endmsg;
    debug() << "~~~ BOOST ~~~" << endmsg;
    debug() << "" << endmsg;

    debug() << "Particle:" << endmsg;
    debug() << "  - PDG ID: " << outParticle.getPDG() << endmsg;
    debug() << "  - mass: " << outParticle.getMass() << endmsg;
    debug() << "  - vertex: " << endmsg;
    debug() << "    - x: " << outParticle.getVertex().x << endmsg;
    debug() << "    - y: " << outParticle.getVertex().y << endmsg;
    debug() << "    - z: " << outParticle.getVertex().z << endmsg;
    debug() << "  - momentum: " << endmsg;
    debug() << "    - px: " << outParticle.getMomentum().x << endmsg;
    debug() << "    - py: " << outParticle.getMomentum().y << endmsg;
    debug() << "    - pz: " << outParticle.getMomentum().z << endmsg;
    debug() << "  - energy: " << eb << endmsg;

    outParticles->push_back(outParticle);
  }

  debug() << "Output particle collection size: " << outParticles->size()
          << endmsg;

  return StatusCode::SUCCESS;
}


StatusCode SimG4CrossingAngleBoost::finalize() {
  return GaudiAlgorithm::finalize();
}
