#ifndef SIMG4COMMON_MAPFIELD3DREGULAR_H
#define SIMG4COMMON_MAPFIELD3DREGULAR_H

// Geant 4
#include "G4MagneticField.hh"

/** @class sim::MapField3DRegular SimG4Common/SimG4Common/MapField3DRegular.h MapField3DRegular.h
*
*  Magnetic field from the field map.
*  Regularly spaced 3D map is expected.
*
*  @author Juraj Smiesko
*/

namespace sim {
  class MapField3DRegular : public G4MagneticField {
    public:
    // Constructor
    explicit MapField3DRegular(const std::vector<double>& bX,
                               const std::vector<double>& bY,
                               const std::vector<double>& bZ,
                               const std::vector<double>& posX,
                               const std::vector<double>& posY,
                               const std::vector<double>& posZ);
    // Destructor
    virtual ~MapField3DRegular() {}

    /// Get the value of the magnetic field value at position
    /// @param[in] point the position where the field is to be returned
    /// @param[out] bField the return value
    virtual void GetFieldValue(const G4double point[4], double* bField) const final;

  private:
    /// Bx component of the field
    std::vector<std::vector<std::vector<double>>> m_fieldX;
    /// By component of the field
    std::vector<std::vector<std::vector<double>>> m_fieldY;
    /// Bz component of the field
    std::vector<std::vector<std::vector<double>>> m_fieldZ;
    /// Extend of the field in x direction
    double m_minX, m_maxX, m_widthX;
    /// Extend of the field in y direction
    double m_minY, m_maxY, m_widthY;
    /// Extend of the field in z direction
    double m_minZ, m_maxZ, m_widthZ;
    /// Number of datapoints in every direction
    size_t m_nX, m_nY, m_nZ;
  };
}
#endif /* SIMG4COMMON_MAPFIELD3DREGULAR_H */
