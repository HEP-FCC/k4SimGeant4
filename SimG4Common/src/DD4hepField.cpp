#include "SimG4Common/DD4hepField.h"
// Geant 4
#include "G4SystemOfUnits.hh"

namespace k4simgeant4 {
  DD4hepField::DD4hepField(dd4hep::OverlayedField field) : m_field{field} {}

  void DD4hepField::GetFieldValue(const G4double point[4], double* bField) const {
    static const double lenghtFactor = dd4hep::mm / CLHEP::mm;
    static const double fieldFactor = CLHEP::tesla / dd4hep::tesla;

    const double position[3] = {point[0] * lenghtFactor,
                                point[1] * lenghtFactor,
                                point[2] * lenghtFactor};

    m_field.magneticField(position, bField);

    bField[0] *= fieldFactor;
    bField[1] *= fieldFactor;
    bField[2] *= fieldFactor;
  }

  G4bool DD4hepField::DoesFieldChangeEnergy() const {
    return m_field.changesEnergy();
  }
}
