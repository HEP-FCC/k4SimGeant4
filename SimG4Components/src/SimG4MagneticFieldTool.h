#ifndef SIMG4COMPONENTS_G4MAGNETICFIELDTOOL_H
#define SIMG4COMPONENTS_G4MAGNETICFIELDTOOL_H

// Gaudi
#include "GaudiKernel/AlgTool.h"

// k4FWCore
#include "k4Interface/ISimG4MagneticFieldTool.h"
#include "k4Interface/IGeoSvc.h"

// Geant4
#include "G4SystemOfUnits.hh"
#include "G4MagneticField.hh"

// Forward declarations:
// Geant4 classes
class G4MagIntegratorStepper;


/** @class SimG4MagneticFieldTool SimG4Components/src/SimG4MagneticFieldTool.h
*  SimG4MagneticFieldTool.h
*
*  Implementation of ISimG4MagneticFieldTool that propagates magnetic field
*  defined in the DD4hep compact file.
*
*  @author Juraj Smiesko
*  @date   2023-06-21
*/

class SimG4MagneticFieldTool : public AlgTool, virtual public ISimG4MagneticFieldTool {
public:
  /// Standard constructor
  SimG4MagneticFieldTool(const std::string& type,
                         const std::string& name,
                         const IInterface* parent);

  /// Destructor
  virtual ~SimG4MagneticFieldTool();

  /// Initialize method
  virtual StatusCode initialize() final;

  /// Finalize method
  virtual StatusCode finalize() final;

  /// Get the magnetic field
  /// @returns pointer to G4MagneticField
  virtual const G4MagneticField* field() const final;

  /// Get the stepper
  /// @returns pointer to G4MagIntegratorStepper (ownership is transferred to the caller)
  G4MagIntegratorStepper* stepper(const std::string&, G4MagneticField*) const;

private:
  /// Pointer to the geometry service
  ServiceHandle<IGeoSvc> m_geoSvc;

  /// Pointer to the actual Geant4 magnetic field
  G4MagneticField* m_field = nullptr;

  /// Switch to turn field on or off (default is off). Set with property FieldOn
  Gaudi::Property<bool> m_fieldOn{this, "FieldOn", false, "Switch to turn field off"};

  /// Minimum epsilon (relative error of position / momentum, see G4 doc for more details). Set with property
  /// MinimumEpsilon
  Gaudi::Property<double> m_minEps{this, "MinimumEpsilon", 0, "Minimum epsilon (see G4 documentation)"};

  /// Maximum epsilon (relative error of position / momentum, see G4 doc for more details). Set with property
  /// MaximumEpsilon
  Gaudi::Property<double> m_maxEps{this, "MaximumEpsilon", 0, "Maximum epsilon (see G4 documentation)"};

  /// This parameter governs accuracy of volume intersection, see G4 doc for more details. Set with property DeltaChord
  Gaudi::Property<double> m_deltaChord{this, "DeltaChord", 0, "Missing distance for the chord finder"};

  /// This parameter is roughly the position error which is acceptable in an integration step, see G4 doc for details.
  /// Set with property DeltaOneStep
  Gaudi::Property<double> m_deltaOneStep{this, "DeltaOneStep", 0, "Delta(one-step)"};

  /// Upper limit of the step size, see G4 doc for more details. Set with property MaximumStep
  Gaudi::Property<double> m_maxStep{this, "MaximumStep", 1. * m, "Maximum step length in field (see G4 documentation)"};

  /// Lower limit of the step size, see G4 doc for more details. Set with property MinimumStep
  Gaudi::Property<double> m_minStep{this, "MinimumStep", 0.01 * mm, "Minimum step length in field (see G4 documentation)"};

  /// Name of the integration stepper, defaults to NystromRK4.
  Gaudi::Property<std::string> m_integratorStepper{this, "IntegratorStepper", "NystromRK4", "Integrator stepper name"};
};

#endif /* SIMG4COMPONENTS_G4MAGNETICFIELDTOOL_H */
