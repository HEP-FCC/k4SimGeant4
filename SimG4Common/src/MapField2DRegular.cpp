#include "SimG4Common/MapField2DRegular.h"

// Geant 4
#include "G4SystemOfUnits.hh"

/**
 * Regular 2D field map loaded from 4 std::vectors
 * The nodes are expected to be in the middle of the "squares".
 * Inspiration:
 *   https://github.com/iLCSoft/lcgeo/blob/master/detector/other/FieldMapXYZ.cpp
 *   https://gitlab.cern.ch/geant4/geant4/-/blob/master/examples/advanced/purging_magnet/src/PurgMagTabulatedField3D.cc
 */


namespace sim {
  MapField2DRegular::MapField2DRegular(const std::vector<double>& bR,
                                       const std::vector<double>& bZ,
                                       const std::vector<double>& posR,
                                       const std::vector<double>& posZ) {
    // Finding the extend of the map
    m_maxR = *max_element(posR.begin(), posR.end());
    m_minR = *min_element(posR.begin(), posR.end());
    m_widthR = m_maxR - m_minR;

    m_maxZ = *max_element(posZ.begin(), posZ.end());
    m_minZ = *min_element(posZ.begin(), posZ.end());
    m_widthZ = m_maxZ - m_minZ;

    /*
    std::cout << "minR: " << m_minR << "\n";
    std::cout << "maxR: " << m_maxR << "\n";
    std::cout << "widthR: " << m_widthR << "\n";

    std::cout << "minZ: " << m_minZ << "\n";
    std::cout << "maxZ: " << m_maxZ << "\n";
    std::cout << "widthZ: " << m_widthZ << "\n";
    */

    // Guessing the step size of the map
    double stepR = -1.;
    for (size_t i = 0; i < posR.size() - 1; ++i) {
      if (stepR < 0 && (posR.at(i) != posR.at(i + 1))) {
        stepR = std::fabs(posR.at(i) - posR.at(i + 1));
        break;
      }
    }
    double stepZ = -1.;
    for (size_t i = 0; i < posZ.size() - 1; ++i) {
      if (stepZ < 0 && (posZ.at(i) != posZ.at(i + 1))) {
        stepZ = std::fabs(posZ.at(i) - posZ.at(i + 1));
        break;
      }
    }

    // Determining number of nodes on each axis
    m_nR = std::round(((m_maxR - m_minR) / stepR) + 1);
    m_nZ = std::round(((m_maxZ - m_minZ) / stepZ) + 1);

    /*
    std::cout << "r Step: " << stepR << "\n";
    std::cout << "z Step: " << stepZ << "\n";

    std::cout << "n pos R: " << m_nR << "\n";
    std::cout << "n pos Z: " << m_nZ << "\n";
    */

    // Preparing the map with all zeroes
    m_fieldR.resize(m_nR);
    m_fieldZ.resize(m_nR);
    for (size_t i = 0; i < m_nR; ++i) {
      m_fieldR[i].resize(m_nZ, 0.);
      m_fieldZ[i].resize(m_nZ, 0.);
    }

    // Filling the map
    for (size_t index = 0; index < posR.size(); ++index) {
      size_t i = (posR.at(index) - m_minR) * (m_nR - 1) / m_widthR;
      size_t j = (posZ.at(index) - m_minZ) * (m_nZ - 1) / m_widthZ;
      m_fieldR[i][j] = bR.at(index);
      m_fieldZ[i][j] = bZ.at(index);
      /*
      std::cout << "i, j: " << i << ", " << j << "\n"
                << "index: " << index << "\n"
                << "r, z: " << posR.at(index) << ", "
                            << posZ.at(index) << "\n"
                << "bR: " << bR.at(index) << "\n"
                << "bZ: " << bZ.at(index) << "\n";
      */
    }
  }

  void MapField2DRegular::GetFieldValue(const G4double point[4], double* bField) const {
    double x = point[0];
    double y = point[1];
    double z = point[2];

    double r = std::sqrt(std::pow(x, 2) + std::pow(y, 2));

    if (r <= m_maxR && z >= m_minZ && z <= m_maxZ) {
      double fractionR = (r - m_minR) / m_widthR;
      if (fractionR < 0.) {
        fractionR = 0.;
      }
      double fractionZ = (z - m_minZ) / m_widthZ;

      double indexRDbl, indexZDbl;
      double localR = std::modf(fractionR * (m_nR - 1), &indexRDbl);
      double localZ = std::modf(fractionZ * (m_nZ - 1), &indexZDbl);

      int indexR = static_cast<int>(indexRDbl);
      int indexZ = static_cast<int>(indexZDbl);

      double bFieldR =
        m_fieldR[indexR  ][indexZ  ] * (1-localR) * (1-localZ) +
        m_fieldR[indexR  ][indexZ+1] * (1-localR) *    localZ  +
        m_fieldR[indexR+1][indexZ  ] *    localR  * (1-localZ) +
        m_fieldR[indexR+1][indexZ+1] *    localR  *    localZ;

      double bFieldZ =
        m_fieldZ[indexR  ][indexZ  ] * (1-localR) * (1-localZ) +
        m_fieldZ[indexR  ][indexZ+1] * (1-localR) *    localZ  +
        m_fieldZ[indexR+1][indexZ  ] *    localR  * (1-localZ) +
        m_fieldZ[indexR+1][indexZ+1] *    localR  *    localZ;

      double phi;
      if (x == 0. && y == 0.) {
        phi = 0.;
      } else if (x != 0) {
        phi = std::atan2(y, x);
      } else if (y == 0) {
        phi = 0.;
      } else if (y >  0) {
        phi = CLHEP::pi / 2;
      } else {
        phi = -CLHEP::pi / 2;
      }

      bField[0] = bFieldR * std::cos(phi);
      bField[1] = bFieldR * std::sin(phi);
      bField[2] = bFieldZ;

      /*
      std::cout << "---------------------------\n";
      std::cout << "x: " << x << "\n";
      std::cout << "y: " << y << "\n";
      std::cout << "z: " << z << "\n";
      std::cout << "r: " << r << "\n";
      std::cout << "phi: " << phi << "\n";
      std::cout << "fractionR: " << fractionR << "\n";
      std::cout << "fractionZ: " << fractionZ << "\n";
      std::cout << "Local r,z: " << localR << " " << localZ << "\n";
      std::cout << "Index r,z: " << indexR << " " << indexZ << "\n";
      std::cout << "Br, Bz: " << m_fieldR[indexR][indexZ] << " " << m_fieldZ[indexR][indexZ] << "\n";
      std::cout << "Br+1, Bz: " << m_fieldR[indexR+1][indexZ] << " " << m_fieldZ[indexR+1][indexZ] << "\n";
      std::cout << "Br, Bz+1: " << m_fieldR[indexR][indexZ+1] << " " << m_fieldZ[indexR][indexZ+1] << "\n";
      std::cout << "Br+1, Bz+1: " << m_fieldR[indexR+1][indexZ+1] << " " << m_fieldZ[indexR+1][indexZ+1] << "\n";
      std::cout << "Br, Bz: " << bFieldR << " " << bFieldZ << "\n";
      std::cout << "Bx, By, Bz: " << bField[0] << ", " << bField[1] << ", " << bField[2] << "\n";
      */
    } else {
      bField[0] = 0.;
      bField[1] = 0.;
      bField[2] = 0.;
    }
  }
}
