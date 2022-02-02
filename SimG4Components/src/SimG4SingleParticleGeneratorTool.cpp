// local
#include "SimG4SingleParticleGeneratorTool.h"

// FCCSW
#include "SimG4Common/Units.h"

// Gaudi
#include "GaudiKernel/PhysicalConstants.h"

// CLHEP
#include <CLHEP/Random/RandFlat.h>

// Geant4
#include "G4Event.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"

// datamodel
#include "edm4hep/MCParticleCollection.h"

// Declaration of the Tool
DECLARE_COMPONENT(SimG4SingleParticleGeneratorTool)

SimG4SingleParticleGeneratorTool::SimG4SingleParticleGeneratorTool(const std::string& type,
                                                                   const std::string& nam,
                                                                   const IInterface* parent)
    : GaudiTool(type, nam, parent) {
  declareInterface<ISimG4EventProviderTool>(this);
  declareProperty("GenParticles", m_genParticlesHandle, "Handle for the genparticles to be written");
}

SimG4SingleParticleGeneratorTool::~SimG4SingleParticleGeneratorTool() {}

StatusCode SimG4SingleParticleGeneratorTool::initialize() {
  if (GaudiTool::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  if (!G4ParticleTable::GetParticleTable()->contains(m_particleName.value())) {
    error() << "Particle " << m_particleName << " cannot be found in G4ParticleTable" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_energyMin > m_energyMax) {
    error() << "Maximum energy cannot be lower than the minumum energy" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_etaMin > m_etaMax) {
    error() << "Maximum psudorapidity cannot be lower than the minumum psudorapidity" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_phiMin > m_phiMax) {
    error() << "Maximum azimuthal angle cannot be lower than the minumum angle" << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

G4Event* SimG4SingleParticleGeneratorTool::g4Event() {
  auto theEvent = new G4Event();
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4ParticleDefinition* particleDef = particleTable->FindParticle(m_particleName.value());
  debug() << "particle definition " << particleDef << " +++ " << m_particleName << endmsg;
  G4double mass = particleDef->GetPDGMass();
  debug() << "particle mass = " << mass << endmsg;

  double particleEnergy = CLHEP::RandFlat::shoot(m_energyMin, m_energyMax);

  debug() << "particle energy = " << particleEnergy << endmsg;

  double eta = CLHEP::RandFlat::shoot(m_etaMin, m_etaMax);
  double phi = CLHEP::RandFlat::shoot(m_phiMin, m_phiMax);

  debug() << "particle eta, phi  = " << eta << " " << phi << endmsg;

  double theta = std::atan(std::exp(-eta)) * 2.;
  double randomX = std::sin(theta) * std::cos(phi);
  double randomY = std::sin(theta) * std::sin(phi);
  double randomZ = std::cos(theta);

  G4ThreeVector particleDir = G4ThreeVector(randomX, randomY, randomZ);
  G4ThreeVector particlePosition = G4ThreeVector(CLHEP::RandFlat::shoot(-m_vertexX, m_vertexX),
                                                 CLHEP::RandFlat::shoot(-m_vertexY, m_vertexY),
                                                 CLHEP::RandFlat::shoot(-m_vertexZ, m_vertexZ));

  G4PrimaryVertex* vertex = new G4PrimaryVertex(particlePosition, 0.);
  G4PrimaryParticle* part = new G4PrimaryParticle(particleDef);

  part->SetMass(mass);
  part->SetKineticEnergy(particleEnergy);
  part->SetMomentumDirection(particleDir);
  part->SetCharge(particleDef->GetPDGCharge());

  vertex->SetPrimary(part);
  theEvent->AddPrimaryVertex(vertex);
  if (m_saveEdm) {
    saveToEdm(vertex, part).ignore();
  }
  return theEvent;
}

StatusCode SimG4SingleParticleGeneratorTool::saveToEdm(const G4PrimaryVertex* aVertex,
                                                       const G4PrimaryParticle* aParticle) {
  edm4hep::MCParticleCollection* particles = new edm4hep::MCParticleCollection();
  auto particle = particles->create();
  particle.setVertex({
       aVertex->GetX0() * sim::g42edm::length,
       aVertex->GetY0() * sim::g42edm::length,
       aVertex->GetZ0() * sim::g42edm::length,
      });
  particle.setTime(aVertex->GetT0() * Gaudi::Units::c_light * sim::g42edm::length);

  particle.setPDG(aParticle->GetPDGcode());
  particle.setGeneratorStatus(1);
  particle.setMomentum({
               (float) (aParticle->GetPx() * sim::g42edm::energy),
               (float) (aParticle->GetPy() * sim::g42edm::energy),
               (float) (aParticle->GetPz() * sim::g42edm::energy),
    });
  particle.setMass(aParticle->GetMass() * sim::g42edm::energy);

  m_genParticlesHandle.put(particles);
  return StatusCode::SUCCESS;
}
