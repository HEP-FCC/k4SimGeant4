// local
#include "SimG4MagneticFieldFromMapTool.h"

// STD
#include <string>
#include <fstream>

// FCCSW
#include "SimG4Common/MapField3DRegular.h"
#include "SimG4Common/MapField2DRegular.h"

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
  if (!sc.isSuccess()) {
    return sc;
  }

  if (!m_fieldOn) {
    return StatusCode::SUCCESS;
  }

  if (m_mapFilePath.empty()) {
    error() << "Input map file not specified!" << endmsg;
    return StatusCode::FAILURE;
  }

  if(gSystem->AccessPathName(m_mapFilePath.value().c_str())) {
    error() << "Fieldmap file does not exist!" << endmsg;
    error() << "    " << m_mapFilePath.value() << endmsg;
    return StatusCode::FAILURE;
  }

  if (m_mapFilePath.value().find(".root") != std::string::npos) {
    sc = loadRootMap();
    if (!sc.isSuccess()) {
      return sc;
    }
  } else if (m_mapFilePath.value().find(".txt") != std::string::npos) {
    sc = loadComsolMap();
    if (!sc.isSuccess()) {
      return sc;
    }
  } else {
    error() << "Fieldmap file extension not recognized!" << endmsg;
    error() << "    " << m_mapFilePath.value() << endmsg;
    return StatusCode::FAILURE;
  }

  G4TransportationManager* transpManager = G4TransportationManager::GetTransportationManager();
  G4FieldManager* fieldManager = transpManager->GetFieldManager();
  G4PropagatorInField* propagator = transpManager->GetPropagatorInField();

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

  return StatusCode::SUCCESS;
}


StatusCode SimG4MagneticFieldFromMapTool::finalize() {
  StatusCode sc = GaudiTool::finalize();

  return sc;
}


const G4MagneticField* SimG4MagneticFieldFromMapTool::field() const {
  return m_field;
}


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


StatusCode SimG4MagneticFieldFromMapTool::loadRootMap() {
  std::unique_ptr<TFile> inFile(TFile::Open(m_mapFilePath.value().c_str(), "READ"));
  if (inFile->IsZombie()) {
    error() << "Can't open the file with fieldmap!" << endmsg;
    error() << "    " << m_mapFilePath.value() << endmsg;
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

  std::vector<double> fieldComponentX;
  std::vector<double> fieldComponentY;
  std::vector<double> fieldComponentZ;
  std::vector<double> fieldPositionX;
  std::vector<double> fieldPositionY;
  std::vector<double> fieldPositionZ;

  int nEntries = inTree->GetEntries();
  for (int i = 0; i < nEntries; ++i) {
    inTree->GetEntry(i);
    fieldPositionX.emplace_back(x * millimeter);
    fieldPositionY.emplace_back(y * millimeter);
    fieldPositionZ.emplace_back(z * millimeter);
    fieldComponentX.emplace_back(bx * tesla);
    fieldComponentY.emplace_back(by * tesla);
    fieldComponentZ.emplace_back(bz * tesla);
  }
  debug() << "Loaded map with " << fieldPositionX.size() << " nodes."
          << endmsg;
  if (fieldComponentX.size() < 1) {
    error() << "Could not load any mapfield nodes!" << endmsg;
  }

  m_field = new sim::MapField3DRegular(fieldComponentX,
                                       fieldComponentY,
                                       fieldComponentZ,
                                       fieldPositionX,
                                       fieldPositionY,
                                       fieldPositionZ);

  return StatusCode::SUCCESS;
}


StatusCode SimG4MagneticFieldFromMapTool::loadComsolMap() {
  std::ifstream inFile;
  inFile.open(m_mapFilePath.value());

  if (!inFile.is_open()) {
    error() << "Can't open the file with fieldmap!" << endmsg;
    error() << "    " << m_mapFilePath.value() << endmsg;
    return StatusCode::FAILURE;
  }
  debug() << "Loading magnetic field map from file: " << endmsg;
  debug() << "    " << m_mapFilePath.value() << endmsg;

  std::string inLine;
  size_t nLinesExpected;
  std::vector<double> fieldPositionR;
  std::vector<double> fieldPositionZ;
  std::vector<double> fieldComponentR;
  std::vector<double> fieldComponentZ;
  while(getline(inFile, inLine)) {
    if (inLine.empty()) {
      continue;
    }

    std::istringstream inLineStream(inLine);

    inLineStream >> std::ws;
    char c = inLineStream.peek();
    if (c == '%') {
      inLineStream.get();
      std::string key, val;
      inLineStream >> key >> val;
      if (key == "Dimension:") {
        int nDim = std::stoi(val);
        if (nDim != 2) {
          error() << "Expected 2D map, got map with " << val << " dimensions!"
                  << endmsg;
          return StatusCode::FAILURE;
        }
      }
      if (key == "Nodes:") {
        nLinesExpected = std::stoi(val);
      }
      continue;
    }
    // debug() << nLines << ": " << inLine << endmsg;

    double r, z, Br, Bphi, Bz, normB;
    inLineStream >> r >> z >> Br >> Bphi >> Bz >> normB;
    /*
    r *= 100;
    r = std::round(r);
    r /= 100;
    z *= 1000;
    z = std::round(z);
    z /= 1000;
    */
    fieldPositionR.emplace_back(r * meter);
    fieldPositionZ.emplace_back(z * meter);
    fieldComponentR.emplace_back(Br * tesla);
    fieldComponentZ.emplace_back(Bz * tesla);
  }

  if (fieldPositionR.size() != nLinesExpected) {
    error() << "Expected " << nLinesExpected << " nodes, got: "
            << fieldPositionR.size() << endmsg;
    return StatusCode::FAILURE;
  }

  inFile.close();

  debug() << "Loaded map with " << fieldPositionR.size() << " nodes." << endmsg;
  if (fieldComponentR.size() < 1) {
    error() << "Could not load any mapfield nodes!" << endmsg;
  }

  m_field = new sim::MapField2DRegular(fieldComponentR,
                                       fieldComponentZ,
                                       fieldPositionR,
                                       fieldPositionZ);

  return StatusCode::SUCCESS;
}
