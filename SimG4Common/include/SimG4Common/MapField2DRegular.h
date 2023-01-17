#ifndef SIMG4COMMON_MAPFIELD2DREGULAR_H
#define SIMG4COMMON_MAPFIELD2DREGULAR_H

// Geant 4
#include "G4MagneticField.hh"

/** @class sim::MapField2DRegular SimG4Common/SimG4Common/MapField2DRegular.h MapField2DRegular.h
*
*  Magnetic field from the COMSOL field map.
*  The Radially symmetric regularly spaced map is expected.
*
*  @author Juraj Smiesko
*/

namespace sim {
  class MapField2DRegular : public G4MagneticField {
    public:
    // Constructor
    explicit MapField2DRegular(const std::vector<double>& bR,
                               const std::vector<double>& bZ,
                               const std::vector<double>& posR,
                               const std::vector<double>& posZ);
    // Destructor
    virtual ~MapField2DRegular() {}

    /// Get the value of the magnetic field value at position
    /// @param[in] point the position where the field is to be returned
    /// @param[out] bField the return value
    virtual void GetFieldValue(const G4double point[4], double* bField) const final;

  private:
    /// Br component of the field
    std::vector<std::vector<double>> m_fieldR;
    /// Bz component of the field
    std::vector<std::vector<double>> m_fieldZ;
    /// Extend of the field in r direction
    double m_minR, m_maxR, m_widthR;
    /// Extend of the field in z direction
    double m_minZ, m_maxZ, m_widthZ;
    /// Number of datapoints in every direction
    size_t m_nR, m_nZ;
  };
}

#endif /* SIMG4COMMON_MAPFIELD2DREGULAR_H */
