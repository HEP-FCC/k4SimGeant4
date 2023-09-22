#ifndef SimG4OpticalPhysicsList_h
#define SimG4OpticalPhysicsList_h 1

// Gaudi
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// FCCSW
#include "k4Interface/ISimG4PhysicsList.h"

class SimG4OpticalPhysicsList : public AlgTool, virtual public ISimG4PhysicsList {
public:
  explicit SimG4OpticalPhysicsList(const std::string& aType, const std::string& aName, const IInterface* aParent);
  virtual ~SimG4OpticalPhysicsList();

  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual G4VModularPhysicsList* physicsList();
  Gaudi::Property<bool> SetCerenkov{this, "SetCerenkov", true, "Bool variable that enables Cerenkov process. Default true."};
  Gaudi::Property<bool> SetScintillation{this, "SetScintillation", true, "Bool variable that enables Scintillation process. Default true."};
  Gaudi::Property<bool> SetTransitionRadiation{this, "SetTransitionRadiation", false, "Bool variable that enables transition_radiation process. Default false."};
private:
  /// Handle for the full physics list tool
  ToolHandle<ISimG4PhysicsList> m_physicsListTool{"SimG4FtfpBert", this, true};
};

#endif
