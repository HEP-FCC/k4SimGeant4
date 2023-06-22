#ifndef SIMG4COMMON_DD4HEPFIELD_H
#define SIMG4COMMON_DD4HEPFIELD_H

// DD4hep
#include "DD4hep/Detector.h"

// Geant 4
#include "G4MagneticField.hh"

/** @class k4simgeant4::DD4hepField SimG4Common/SimG4Common/DD4hepField.h DD4hepField.h
*
*  Mediator class between DD4hep overlayed field and Geant4 magnetic field.
*
*  @author Juraj Smiesko
*/

namespace k4simgeant4 {
  class DD4hepField : public G4MagneticField {
    public:
      /// Constructor with field required
      explicit DD4hepField(dd4hep::OverlayedField field);
      // Destructor
      virtual ~DD4hepField() {}

      /// Get the value of the magnetic field value at position
      /// @param[in] point the position where the field is to be returned
      /// @param[out] bField the return value
      virtual void GetFieldValue(const G4double point[4], double* bField) const final;

      /// Does field change energy ?
      virtual G4bool DoesFieldChangeEnergy() const;

    private:
      /// DD4hep OverlayedField
      dd4hep::OverlayedField m_field;
  };
}
#endif /* SIMG4COMMON_CONSTANTFIELD_H */
