#include "EnergyInCaloLayers.h"

// std
#include <cstddef>
#include <podio/UserDataCollection.h>

// DD4hep
#include "DD4hep/Detector.h"
#include "DD4hep/Readout.h"


DECLARE_COMPONENT(EnergyInCaloLayers)


EnergyInCaloLayers::EnergyInCaloLayers(const std::string& aName, ISvcLocator* aSvcLoc)
    : GaudiAlgorithm(aName, aSvcLoc), m_geoSvc("GeoSvc", aName) {
  declareProperty("deposits", m_deposits, "Energy deposits (input)");
  declareProperty("particle", m_particle, "Generated single-particle event (input)");

  declareProperty("energyInLayer", m_energyInLayer, "Energy deposited in every calorimeter layer (output)");
  declareProperty("energyInCryo", m_energyInCryo, "Energy deposited in cryostat parts (output)");
  declareProperty("particleVec", m_particleVec, "Initial particle momentum vector (output)");
}


EnergyInCaloLayers::~EnergyInCaloLayers() {}


StatusCode EnergyInCaloLayers::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) return StatusCode::FAILURE;

  // Check geometry service
  if (!m_geoSvc) {
    error() << "Unable to locate Geometry Service! "
            << "Make sure you have GeoSvc and SimSvc in the right order in the configuration." << endmsg;
    return StatusCode::FAILURE;
  }

  // Check if readout exists
  if (m_geoSvc->getDetector()->readouts().find(m_readoutName) == m_geoSvc->getDetector()->readouts().end()) {
    error() << "Can't find readout <<" << m_readoutName << ">>!" << endmsg;
    return StatusCode::FAILURE;
  }

  // Check if number of layers matches number of sampling fractions
  if (m_samplingFractions.size() != m_numLayers) {
    error() << "Number of sampling fractions does not match number of calorimeter layers!" << endmsg;
    return StatusCode::FAILURE;
  }

  // Check if there are no zeros in sampling fractions array
  for (auto& samplingFraction: m_samplingFractions) {
    if (samplingFraction == 0.) {
      error() << "One of the sampling fractions equals to zero!" << endmsg;
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode EnergyInCaloLayers::execute() {
  auto decoder = m_geoSvc->getDetector()->readout(m_readoutName).idSpec().decoder();

  // Initialize output variables
  auto* energyInLayerColl = m_energyInLayer.createAndPut();
  energyInLayerColl->vec().assign(m_numLayers, 0.);
  auto* energyInCryoColl = m_energyInCryo.createAndPut();
  energyInCryoColl->vec().assign(6, 0.);
  auto* particleVecColl = m_particleVec.createAndPut();
  particleVecColl->vec().assign(4, 0.);

  // Fill particle vector
  const auto particles = m_particle.get();
  if (particles->size() == 0) {
    error() << "Initial particle not found!" << endmsg;
    return StatusCode::FAILURE;
  }
  if (particles->size() > 1) {
    warning() << "Found more than one initial particle!" << endmsg;
  }
  particleVecColl->vec().at(0) = particles->at(0).getMass();
  particleVecColl->vec().at(1) = particles->at(0).getMomentum().x;
  particleVecColl->vec().at(2) = particles->at(0).getMomentum().y;
  particleVecColl->vec().at(3) = particles->at(0).getMomentum().z;

  // Calculate particle theta
  double rxy = std::sqrt(std::pow(particles->at(0).getMomentum().x, 2) + std::pow(particles->at(0).getMomentum().y, 2));
  double particleTheta = fabs(std::atan2(rxy, particles->at(0).getMomentum().z));
  particleTheta = 180. * particleTheta / M_PI;

  // Get the energy deposited in the calorimeter layers and in the cryostat and its parts
  const auto deposits = m_deposits.get();
  for (const auto& hit: *deposits) {
    dd4hep::DDSegmentation::CellID cellID = hit.getCellID();
    size_t cryoID = decoder->get(cellID, "cryo");
    if (cryoID == 0) {
      size_t layerID = decoder->get(cellID, "layer");
      energyInLayerColl->vec().at(layerID) += hit.getEnergy();
    } else {
      size_t cryoTypeID = decoder->get(cellID, "type");
      energyInCryoColl->vec().at(0) += hit.getEnergy();
      energyInCryoColl->vec().at(cryoTypeID) += hit.getEnergy();
    }
  }

  // Calibrate energy in the calorimeter layers
  for (size_t i = 0; i < energyInLayerColl->size(); ++i) {
    energyInLayerColl->vec().at(i) /= m_samplingFractions[i];
  }

  // Energy deposited in the whole calorimeter
  double energyInCalo = 0.;
  for (size_t i = 0; i < energyInLayerColl->size(); ++i) {
    energyInCalo += energyInLayerColl->vec().at(i);
  }

  // Printouts
  verbose() << "********************************************************************" << endmsg;
  verbose() << "Particle theta: " << particleTheta << " deg" << endmsg << endmsg;

  verbose() << "Energy in layers:" << endmsg;
  for (size_t i = 0; i < energyInLayerColl->size(); ++i) {
    verbose() << "  * layer " << i << ": " << energyInLayerColl->vec().at(i) << " GeV" << endmsg;
  }
  verbose() << endmsg;

  verbose() << "Energy in calorimeter: " << energyInCalo << " GeV" << endmsg;
  verbose() << "Energy in cryostat: " << energyInCryoColl->vec().at(0) << " GeV" << endmsg;
  verbose() << "Energy in cryostat front: " << energyInCryoColl->vec().at(1) << " GeV" << endmsg;
  verbose() << "Energy in cryostat back: " << energyInCryoColl->vec().at(2) << " GeV" << endmsg;
  verbose() << "Energy in cryostat sides: " << energyInCryoColl->vec().at(3) << " GeV" << endmsg;
  verbose() << "Energy in cryostat LAr bath front: " << energyInCryoColl->vec().at(4) << " GeV" << endmsg;
  verbose() << "Energy in cryostat LAr bath back: " << energyInCryoColl->vec().at(5) << " GeV" << endmsg << endmsg;

  verbose() << "Energy in calorimeter and in the cryostat front: "
            << energyInCalo + energyInCryoColl->vec().at(1) + energyInCryoColl->vec().at(4) << " GeV" << endmsg;
  verbose() << "Energy in calorimeter and in the cryostat back: "
            << energyInCalo + energyInCryoColl->vec().at(2) + energyInCryoColl->vec().at(5) << " GeV" << endmsg;
  verbose() << "Energy in calorimeter and in the cryostat: " << energyInCalo + energyInCryoColl->vec().at(0) << " GeV" << endmsg;

  return StatusCode::SUCCESS;
}


StatusCode EnergyInCaloLayers::finalize() {
  return GaudiAlgorithm::finalize();
}
