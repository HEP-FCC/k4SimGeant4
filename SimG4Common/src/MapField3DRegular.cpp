#include "SimG4Common/MapField3DRegular.h"

// Geant 4
#include "G4SystemOfUnits.hh"
#include <algorithm>

/**
 * Field map loaded from 6 std::vectors.
 * Expects nodes to be at the corners of the "boxes".
 * Inspiration:
 *   https://github.com/iLCSoft/lcgeo/blob/master/detector/other/FieldMapXYZ.cpp
 *   https://gitlab.cern.ch/geant4/geant4/-/blob/master/examples/advanced/purging_magnet/src/PurgMagTabulatedField3D.cc
 */


namespace sim {
  MapField3DRegular::MapField3DRegular(const std::vector<double>& bX,
                                       const std::vector<double>& bY,
                                       const std::vector<double>& bZ,
                                       const std::vector<double>& posX,
                                       const std::vector<double>& posY,
                                       const std::vector<double>& posZ) {
    // Finding the extend of the map
    m_maxX = *std::max_element(posX.begin(), posX.end());
    m_minX = *std::min_element(posX.begin(), posX.end());
    m_widthX = m_maxX - m_minX;

    m_maxY = *std::max_element(posY.begin(), posY.end());
    m_minY = *std::min_element(posY.begin(), posY.end());
    m_widthY = m_maxY - m_minY;

    m_maxZ = *std::max_element(posZ.begin(), posZ.end());
    m_minZ = *std::min_element(posZ.begin(), posZ.end());
    m_widthZ = m_maxZ - m_minZ;

    /*
    std::cout << "X min: " << m_minX << "\n";
    std::cout << "X max: " << m_maxX << "\n";
    std::cout << "X width: " << m_widthX << "\n";

    std::cout << "Y min: " << m_minY << "\n";
    std::cout << "Y max: " << m_maxY << "\n";
    std::cout << "Y width: " << m_widthY << "\n";

    std::cout << "Z min: " << m_minZ << "\n";
    std::cout << "Z max: " << m_maxZ << "\n";
    std::cout << "Z width: " << m_widthZ << "\n";
    */

    // Guessing the step size of the map
    double stepX = -1.;
    for (size_t i = 0; i < posX.size() - 1; ++i) {
      if (stepX < 0 && (posX.at(i) != posX.at(i + 1))) {
        stepX = std::fabs(posX.at(i) - posX.at(i + 1));
        break;
      }
    }
    double stepY = -1.;
    for (size_t i = 0; i < posY.size() - 1; ++i) {
      if (stepY < 0 && (posY.at(i) != posY.at(i + 1))) {
        stepY = std::fabs(posY.at(i) - posY.at(i + 1));
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
    m_nX = std::round(((m_maxX - m_minX) / stepX) + 1);
    m_nY = std::round(((m_maxY - m_minY) / stepY) + 1);
    m_nZ = std::round(((m_maxZ - m_minZ) / stepZ) + 1);

    /*
    std::cout << "x Step: " << stepX << "\n";
    std::cout << "y Step: " << stepY << "\n";
    std::cout << "z Step: " << stepZ << "\n";

    std::cout << "n pos X: " << m_nX << "\n";
    std::cout << "n pos Y: " << m_nY << "\n";
    std::cout << "n pos Z: " << m_nZ << "\n";
    */

    // Preparing the map with all zeroes
    m_fieldX.resize(m_nX);
    m_fieldY.resize(m_nX);
    m_fieldZ.resize(m_nX);
    for (size_t i = 0; i < m_nX; ++i) {
      m_fieldX[i].resize(m_nY);
      m_fieldY[i].resize(m_nY);
      m_fieldZ[i].resize(m_nY);
      for (size_t j = 0; j < m_nY; ++j) {
        m_fieldX[i][j].resize(m_nZ, 0.);
        m_fieldY[i][j].resize(m_nZ, 0.);
        m_fieldZ[i][j].resize(m_nZ, 0.);
      }
    }

    // Filling the map
    for (size_t index = 0; index < posX.size(); ++index) {
      size_t i = (posX.at(index) - m_minX) * (m_nX - 1) / m_widthX;
      size_t j = (posY.at(index) - m_minY) * (m_nY - 1) / m_widthY;
      size_t k = (posZ.at(index) - m_minZ) * (m_nZ - 1) / m_widthZ;
      m_fieldX[i][j][k] = bX.at(index);
      m_fieldY[i][j][k] = bY.at(index);
      m_fieldZ[i][j][k] = bZ.at(index);
      /*
      std::cout << "i, j, k: " << i << ", " << j << ", " << k << "\n"
                << "index: " << index << "\n"
                << "x, y, z: " << posX.at(index) << ", "
                               << posY.at(index) << ", "
                               << posZ.at(index) << "\n"
                << "bX: " << bX.at(index) << "\n"
                << "bY: " << bY.at(index) << "\n"
                << "bZ: " << bZ.at(index) << "\n";
      */
    }
  }

  void MapField3DRegular::GetFieldValue(const G4double point[4], double* bField) const {
    double x = point[0];
    double y = point[1];
    double z = point[2];

    if (x >= m_minX && x <= m_maxX &&
        y >= m_minY && y <= m_maxY &&
        z >= m_minZ && z <= m_maxZ) {

      double fractionX = (x - m_minX) / m_widthX;
      double fractionY = (y - m_minY) / m_widthY;
      double fractionZ = (z - m_minZ) / m_widthZ;

      double indexXDbl, indexYDbl, indexZDbl;
      double localX = std::modf(fractionX * (m_nX - 1), &indexXDbl);
      double localY = std::modf(fractionY * (m_nY - 1), &indexYDbl);
      double localZ = std::modf(fractionZ * (m_nZ - 1), &indexZDbl);

      int indexX = static_cast<int>(indexXDbl);
      int indexY = static_cast<int>(indexYDbl);
      int indexZ = static_cast<int>(indexZDbl);

      /*
      std::cout << "---------------------------\n";
      std::cout << "x: " << x << "\n";
      std::cout << "y: " << y << "\n";
      std::cout << "z: " << z << "\n";
      std::cout << "fractionX: " << fractionX << "\n";
      std::cout << "fractionY: " << fractionY << "\n";
      std::cout << "fractionZ: " << fractionZ << "\n";
      std::cout << "Local x,y,z: " << localX << " " << localY << " " << localZ << "\n";
      std::cout << "Index x,y,z: " << indexX << " " << indexY << " " << indexZ << "\n";
      std::cout << "Bx, By, Bz: " << m_fieldX[indexX][indexY][indexZ] << " " << m_fieldY[indexX][indexY][indexZ] << " " << m_fieldZ[indexX][indexY][indexZ] << "\n";
      std::cout << "Bx, By, Bz (x+1): " << m_fieldX[indexX+1][indexY][indexZ] << " " << m_fieldY[indexX+1][indexY][indexZ] << " " << m_fieldZ[indexX+1][indexY][indexZ] << "\n";
      std::cout << "Bx, By, Bz (y+1): " << m_fieldX[indexX][indexY+1][indexZ] << " " << m_fieldY[indexX][indexY+1][indexZ] << " " << m_fieldZ[indexX][indexY+1][indexZ] << "\n";
      std::cout << "Bx, By, Bz (z+1): " << m_fieldX[indexX][indexY][indexZ+1] << " " << m_fieldY[indexX][indexY][indexZ+1] << " " << m_fieldZ[indexX][indexY][indexZ+1] << "\n";
      std::cout << "Bx, By, Bz (all+1): " << m_fieldX[indexX+1][indexY+1][indexZ+1] << " " << m_fieldY[indexX+1][indexY+1][indexZ+1] << " " << m_fieldZ[indexX+1][indexY+1][indexZ+1] << "\n";
      */

      bField[0] =
        m_fieldX[indexX  ][indexY  ][indexZ  ] * (1-localX) * (1-localY) * (1-localZ) +
        m_fieldX[indexX  ][indexY  ][indexZ+1] * (1-localX) * (1-localY) *    localZ  +
        m_fieldX[indexX  ][indexY+1][indexZ  ] * (1-localX) *    localY  * (1-localZ) +
        m_fieldX[indexX  ][indexY+1][indexZ+1] * (1-localX) *    localY  *    localZ  +
        m_fieldX[indexX+1][indexY  ][indexZ  ] *    localX  * (1-localY) * (1-localZ) +
        m_fieldX[indexX+1][indexY  ][indexZ+1] *    localX  * (1-localY) *    localZ  +
        m_fieldX[indexX+1][indexY+1][indexZ  ] *    localX  *    localY  * (1-localZ) +
        m_fieldX[indexX+1][indexY+1][indexZ+1] *    localX  *    localY  *    localZ;

      bField[1] =
        m_fieldY[indexX  ][indexY  ][indexZ  ] * (1-localX) * (1-localY) * (1-localZ) +
        m_fieldY[indexX  ][indexY  ][indexZ+1] * (1-localX) * (1-localY) *    localZ  +
        m_fieldY[indexX  ][indexY+1][indexZ  ] * (1-localX) *    localY  * (1-localZ) +
        m_fieldY[indexX  ][indexY+1][indexZ+1] * (1-localX) *    localY  *    localZ  +
        m_fieldY[indexX+1][indexY  ][indexZ  ] *    localX  * (1-localY) * (1-localZ) +
        m_fieldY[indexX+1][indexY  ][indexZ+1] *    localX  * (1-localY) *    localZ  +
        m_fieldY[indexX+1][indexY+1][indexZ  ] *    localX  *    localY  * (1-localZ) +
        m_fieldY[indexX+1][indexY+1][indexZ+1] *    localX  *    localY  *    localZ;

      bField[2] =
        m_fieldZ[indexX  ][indexY  ][indexZ  ] * (1-localX) * (1-localY) * (1-localZ) +
        m_fieldZ[indexX  ][indexY  ][indexZ+1] * (1-localX) * (1-localY) *    localZ  +
        m_fieldZ[indexX  ][indexY+1][indexZ  ] * (1-localX) *    localY  * (1-localZ) +
        m_fieldZ[indexX  ][indexY+1][indexZ+1] * (1-localX) *    localY  *    localZ  +
        m_fieldZ[indexX+1][indexY  ][indexZ  ] *    localX  * (1-localY) * (1-localZ) +
        m_fieldZ[indexX+1][indexY  ][indexZ+1] *    localX  * (1-localY) *    localZ  +
        m_fieldZ[indexX+1][indexY+1][indexZ  ] *    localX  *    localY  * (1-localZ) +
        m_fieldZ[indexX+1][indexY+1][indexZ+1] *    localX  *    localY  *    localZ;

      // std::cout << "Bx, By, Bz: " << bField[0] << " " << bField[1] << " " << bField[2] << "\n";

    } else {
      bField[0] = 0.;
      bField[1] = 0.;
      bField[2] = 0.;
    }
  }
}
