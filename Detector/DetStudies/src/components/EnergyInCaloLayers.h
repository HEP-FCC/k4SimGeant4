#ifndef DETSTUDIES_ENERGYINCALOLAYERS_H
#define DETSTUDIES_ENERGYINCALOLAYERS_H

// GAUDI
#include "GaudiAlg/GaudiAlgorithm.h"

// Key4HEP
#include "k4Interface/IGeoSvc.h"
#include "k4FWCore/DataHandle.h"

// EDM4hep & Podio
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "podio/UserDataCollection.h"


/** @class EnergyInCaloLayers EnergyInCaloLayers.h
 *
 *  Sums energy deposited in every calorimeter layer separately, sums also energy deposited in the dead material of the
 *  calorimeter (cryostat).
 *
 *  In order to calculate upstream or downstream energy correction one needs to mark cryostat as sensitive in the
 *  detector XML files. Additionally, for the downstream correction, the thickness of the back cryostat needs to be
 *  enlarged to be unrealistically large (at least one meter) to capture all energy deposited behind the calorimeter.
 *
 *  Based on work done by Anna Zaborowska and Jana Faltova.
 *
 *  @author Juraj Smiesko
 */

class EnergyInCaloLayers : public GaudiAlgorithm {
public:
  explicit EnergyInCaloLayers(const std::string&, ISvcLocator*);
  virtual ~EnergyInCaloLayers();
  /**  Initialize.
   *   @return status code
   */
  virtual StatusCode initialize() final;
  /**  Fills the histograms.
   *   @return status code
   */
  virtual StatusCode execute() final;
  /**  Finalize.
   *   @return status code
   */
  virtual StatusCode finalize() final;

private:
  /// Handle for the energy deposits
  DataHandle<edm4hep::CalorimeterHitCollection> m_deposits{"det/caloDeposits", Gaudi::DataHandle::Reader, this};
  /// Handle for the particle
  DataHandle<edm4hep::MCParticleCollection> m_particle{"det/particles", Gaudi::DataHandle::Reader, this};
  /// Handle for vector with energy deposited in every layer
  DataHandle<podio::UserDataCollection<double>> m_energyInLayer {"energyInLayer", Gaudi::DataHandle::Writer, this};
  /// Handle for vector with energy deposited in cryostat and in its parts
  DataHandle<podio::UserDataCollection<double>> m_energyInCryo {"energyInCryo", Gaudi::DataHandle::Writer, this};
  /// Handle for initial particle vector
  DataHandle<podio::UserDataCollection<double>> m_particleVec {"particleVec", Gaudi::DataHandle::Writer, this};

  /// Pointer to the geometry service
  ServiceHandle<IGeoSvc> m_geoSvc;

  /// Number of layers/cells cells
  Gaudi::Property<size_t> m_numLayers{this, "numLayers", 11, "Number of layers"};
  /// Values of sampling fraction
  Gaudi::Property<std::vector<double>> m_samplingFractions{
      this, "samplingFractions", {}, "Values of sampling fraction per layer"};
  /// Name of the detector readout
  Gaudi::Property<std::string> m_readoutName{this, "readoutName", "", "Name of the readout"};
};

#endif /* DETSTUDIES_ENERGYINCALOLAYERS_H */
