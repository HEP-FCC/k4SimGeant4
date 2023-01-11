// local
#include "SimG4MagneticFieldFromMapTool.h"

// FCCSW
#include "SimG4Common/MapField.h"

// ROOT
#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"

// Geant 4
#include "G4ChordFinder.hh"
#include "G4FieldManager.hh"
#include "G4MagneticField.hh"
#include "G4TransportationManager.hh"
#include "G4MagIntegratorDriver.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

#include "G4ClassicalRK4.hh"
#include "G4HelixExplicitEuler.hh"
#include "G4ExactHelixStepper.hh"
#include "G4HelixImplicitEuler.hh"
#include "G4HelixSimpleRunge.hh"
#include "G4MagIntegratorStepper.hh"
#include "G4Mag_UsualEqRhs.hh"
#include "G4NystromRK4.hh"
#include "G4PropagatorInField.hh"

// Declaration of the Tool
DECLARE_COMPONENT(SimG4MagneticFieldFromMapTool)

SimG4MagneticFieldFromMapTool::SimG4MagneticFieldFromMapTool(const std::string& type, const std::string& name,
                                                             const IInterface* parent)
    : GaudiTool(type, name, parent), m_field(nullptr) {
  declareInterface<ISimG4MagneticFieldTool>(this);
}

SimG4MagneticFieldFromMapTool::~SimG4MagneticFieldFromMapTool() {}

StatusCode SimG4MagneticFieldFromMapTool::initialize() {
  StatusCode sc = GaudiTool::initialize();
  if (sc.isFailure()) return sc;

  if (m_fieldOn) {
    G4TransportationManager* transpManager = G4TransportationManager::GetTransportationManager();
    G4FieldManager* fieldManager = transpManager->GetFieldManager();
    G4PropagatorInField* propagator = transpManager->GetPropagatorInField();

    if (m_mapFilePath.empty()) {
      error() << "Input map file not specified!" << endmsg;
      return StatusCode::FAILURE;
    }

    if(gSystem->AccessPathName(m_mapFilePath.value().c_str())) {
      error() << "Fieldmap file does not exist!" << endmsg;
      error() << "    " << m_mapFilePath.value() << endmsg;
      return StatusCode::FAILURE;
    }

    std::unique_ptr<TFile> inFile(TFile::Open(m_mapFilePath.value().c_str(), "READ"));
    if (inFile->IsZombie()) {
      error() << "Can't open the file with fieldmap!" << endmsg;
      return StatusCode::FAILURE;
    } else {
      debug() << "Loading magnetic field map from file: " << endmsg;
      debug() << "    " << m_mapFilePath.value() << endmsg;
    }

    TTree *inTree = dynamic_cast<TTree*>(inFile->Get("ntuple"));
    float x, y, z;
    float bx, by, bz;
    inTree->SetBranchAddress("X", &x);
    inTree->SetBranchAddress("Y", &y);
    inTree->SetBranchAddress("Z", &z);
    inTree->SetBranchAddress("Bx", &bx);
    inTree->SetBranchAddress("By", &by);
    inTree->SetBranchAddress("Bz", &bz);

    int nEntries = inTree->GetEntries();
    for (int i = 0; i < nEntries; ++i) {
      inTree->GetEntry(i);
      m_fieldPositionX.emplace_back(x);
      m_fieldPositionY.emplace_back(y);
      m_fieldPositionZ.emplace_back(z);
      m_fieldComponentX.emplace_back(bx);
      m_fieldComponentY.emplace_back(by);
      m_fieldComponentZ.emplace_back(bz);
    }
    debug() << "Loaded map with " << m_fieldPositionX.size() << " datapoints."
            << endmsg;
    if (m_fieldComponentX.size() < 1) {
      error() << "No mapfield datapoints loaded!" << endmsg;
    }

    m_field = new sim::MapField(m_fieldComponentX,
                                m_fieldComponentY,
                                m_fieldComponentZ,
                                m_fieldPositionX,
                                m_fieldPositionY,
                                m_fieldPositionZ);
    fieldManager->SetDetectorField(m_field);

    G4ChordFinder* chordFinder = new G4ChordFinder(m_field,
                                                   m_minStep,
                                                   stepper(m_integratorStepper,
                                                           m_field));
    fieldManager->SetChordFinder(chordFinder);

    propagator->SetLargestAcceptableStep(m_maxStep);

    if (m_deltaChord > 0) fieldManager->GetChordFinder()->SetDeltaChord(m_deltaChord);
    if (m_deltaOneStep > 0) fieldManager->SetDeltaOneStep(m_deltaOneStep);
    if (m_minEps > 0) fieldManager->SetMinimumEpsilonStep(m_minEps);
    if (m_maxEps > 0) fieldManager->SetMaximumEpsilonStep(m_maxEps);
  }

  return sc;
}

StatusCode SimG4MagneticFieldFromMapTool::finalize() {
  StatusCode sc = GaudiTool::finalize();

  return sc;
}

const G4MagneticField* SimG4MagneticFieldFromMapTool::field() const { return m_field; }

G4MagIntegratorStepper* SimG4MagneticFieldFromMapTool::stepper(const std::string& name, G4MagneticField* field) const {
  G4Mag_UsualEqRhs* fEquation = new G4Mag_UsualEqRhs(field);
  if (name == "HelixImplicitEuler")
    return new G4HelixImplicitEuler(fEquation);
  else if (name == "HelixSimpleRunge")
    return new G4HelixSimpleRunge(fEquation);
  else if (name == "HelixExplicitEuler")
    return new G4HelixExplicitEuler(fEquation);
  else if (name == "NystromRK4")
    return new G4NystromRK4(fEquation);
  else if (name == "ClassicalRK4")
    return new G4ClassicalRK4(fEquation);
  else if (name == "ExactHelix")
    return new G4ExactHelixStepper(fEquation);
  else {
    error() << "Stepper " << name << " not available! returning NystromRK4!" << endmsg;
    return new G4NystromRK4(fEquation);
  }
}
