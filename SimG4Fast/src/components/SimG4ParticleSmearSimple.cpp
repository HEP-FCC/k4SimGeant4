#include "SimG4ParticleSmearSimple.h"

// CLHEP
#include "CLHEP/Vector/ThreeVector.h"

DECLARE_COMPONENT(SimG4ParticleSmearSimple)

SimG4ParticleSmearSimple::SimG4ParticleSmearSimple(const std::string& type, const std::string& name,
                                                   const IInterface* parent)
    : AlgTool(type, name, parent) {
  declareInterface<ISimG4ParticleSmearTool>(this);
}

SimG4ParticleSmearSimple::~SimG4ParticleSmearSimple() {}

StatusCode SimG4ParticleSmearSimple::initialize() {

  m_randSvc = service("RndmGenSvc", false);
  if (!m_randSvc) {
    error() << "Couldn't get RndmGenSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  if (AlgTool::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  m_gauss.initialize(m_randSvc, Rndm::Gauss(1, m_sigma)).ignore();
  info() << "Tool used for smearing particles initialized with constant sigma = " << m_sigma << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode SimG4ParticleSmearSimple::finalize() { return AlgTool::finalize(); }

StatusCode SimG4ParticleSmearSimple::smearMomentum(CLHEP::Hep3Vector& aMom, int /*aPdg*/) {
  double tmp = m_gauss.shoot();
  aMom *= tmp;
  return StatusCode::SUCCESS;
}
