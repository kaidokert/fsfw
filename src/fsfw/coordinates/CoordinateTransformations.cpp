#include "fsfw/coordinates/CoordinateTransformations.h"

#include <cmath>
#include <cstddef>

#include "fsfw/globalfunctions/constants.h"
#include "fsfw/globalfunctions/math/MatrixOperations.h"
#include "fsfw/globalfunctions/math/VectorOperations.h"

void CoordinateTransformations::positionEcfToEci(const double* ecfPosition, double* eciPosition,
                                                 timeval* timeUTC) {
  ecfToEci(ecfPosition, eciPosition, NULL, timeUTC);
}

void CoordinateTransformations::velocityEcfToEci(const double* ecfVelocity,
                                                 const double* ecfPosition, double* eciVelocity,
                                                 timeval* timeUTC) {
  ecfToEci(ecfVelocity, eciVelocity, ecfPosition, timeUTC);
}

void CoordinateTransformations::positionEciToEcf(const double* eciCoordinates,
                                                 double* ecfCoordinates, timeval* timeUTC) {
  eciToEcf(eciCoordinates, ecfCoordinates, NULL, timeUTC);
};

void CoordinateTransformations::velocityEciToEcf(const double* eciVelocity,
                                                 const double* eciPosition, double* ecfVelocity,
                                                 timeval* timeUTC) {
  eciToEcf(eciVelocity, ecfVelocity, eciPosition, timeUTC);
}

double CoordinateTransformations::getEarthRotationAngle(timeval timeUTC) {
  double jD2000UTC;
  Clock::convertTimevalToJD2000(timeUTC, &jD2000UTC);

  double TTt2000 = getJuleanCenturiesTT(timeUTC);

  double theta = 2 * Math::PI * (0.779057273264 + 1.00273781191135448 * jD2000UTC);

  // Correct theta according to IAU 2000 precession-nutation model
  theta = theta + 7.03270725817493E-008 + 0.0223603701 * TTt2000 +
          6.77128219501896E-006 * TTt2000 * TTt2000 +
          4.5300990362875E-010 * TTt2000 * TTt2000 * TTt2000 +
          9.12419347848147E-011 * TTt2000 * TTt2000 * TTt2000 * TTt2000;
  return theta;
}

void CoordinateTransformations::getEarthRotationMatrix(timeval timeUTC, double matrix[][3]) {
  double theta = getEarthRotationAngle(timeUTC);

  matrix[0][0] = cos(theta);
  matrix[0][1] = sin(theta);
  matrix[0][2] = 0;
  matrix[1][0] = -sin(theta);
  matrix[1][1] = cos(theta);
  matrix[1][2] = 0;
  matrix[2][0] = 0;
  matrix[2][1] = 0;
  matrix[2][2] = 1;
}

void CoordinateTransformations::ecfToEci(const double* ecfCoordinates, double* eciCoordinates,
                                         const double* ecfPositionIfCoordinatesAreVelocity,
                                         timeval* timeUTCin) {
  timeval timeUTC;
  if (timeUTCin != NULL) {
    timeUTC = *timeUTCin;
  } else {
    Clock::getClock_timeval(&timeUTC);
  }

  double Tfi[3][3];
  double Tif[3][3];
  getTransMatrixECITOECF(timeUTC, Tfi);

  MatrixOperations<double>::transpose(Tfi[0], Tif[0], 3);

  MatrixOperations<double>::multiply(Tif[0], ecfCoordinates, eciCoordinates, 3, 3, 1);

  if (ecfPositionIfCoordinatesAreVelocity != NULL) {
    double Tdotfi[3][3];
    double Tdotif[3][3];
    double Trot[3][3] = {{0, Earth::OMEGA, 0}, {0 - Earth::OMEGA, 0, 0}, {0, 0, 0}};

    MatrixOperations<double>::multiply(Trot[0], Tfi[0], Tdotfi[0], 3, 3, 3);

    MatrixOperations<double>::transpose(Tdotfi[0], Tdotif[0], 3);

    double velocityCorrection[3];

    MatrixOperations<double>::multiply(Tdotif[0], ecfPositionIfCoordinatesAreVelocity,
                                       velocityCorrection, 3, 3, 1);

    VectorOperations<double>::add(velocityCorrection, eciCoordinates, eciCoordinates, 3);
  }
}

double CoordinateTransformations::getJuleanCenturiesTT(timeval timeUTC) {
  timeval timeTT;
  Clock::convertUTCToTT(timeUTC, &timeTT);
  double jD2000TT;
  Clock::convertTimevalToJD2000(timeTT, &jD2000TT);

  return jD2000TT / 36525.;
}

void CoordinateTransformations::eciToEcf(const double* eciCoordinates, double* ecfCoordinates,
                                         const double* eciPositionIfCoordinatesAreVelocity,
                                         timeval* timeUTCin) {
  timeval timeUTC;
  if (timeUTCin != NULL) {
    timeUTC = *timeUTCin;
  } else {
    Clock::getClock_timeval(&timeUTC);
  }

  double Tfi[3][3];

  getTransMatrixECITOECF(timeUTC, Tfi);

  MatrixOperations<double>::multiply(Tfi[0], eciCoordinates, ecfCoordinates, 3, 3, 1);

  if (eciPositionIfCoordinatesAreVelocity != NULL) {
    double Tdotfi[3][3];
    double Trot[3][3] = {{0, Earth::OMEGA, 0}, {0 - Earth::OMEGA, 0, 0}, {0, 0, 0}};

    MatrixOperations<double>::multiply(Trot[0], Tfi[0], Tdotfi[0], 3, 3, 3);

    double velocityCorrection[3];

    MatrixOperations<double>::multiply(Tdotfi[0], eciPositionIfCoordinatesAreVelocity,
                                       velocityCorrection, 3, 3, 1);

    VectorOperations<double>::add(ecfCoordinates, velocityCorrection, ecfCoordinates, 3);
  }
};

void CoordinateTransformations::getTransMatrixECITOECF(timeval timeUTC, double Tfi[3][3]) {
  double TTt2000 = getJuleanCenturiesTT(timeUTC);

  //////////////////////////////////////////////////////////
  // Calculate Precession Matrix

  double zeta = 0.0111808609 * TTt2000 + 1.46355554053347E-006 * TTt2000 * TTt2000 +
                8.72567663260943E-008 * TTt2000 * TTt2000 * TTt2000;
  double theta_p = 0.0097171735 * TTt2000 - 2.06845757045384E-006 * TTt2000 * TTt2000 -
                   2.02812107218552E-007 * TTt2000 * TTt2000 * TTt2000;
  double z =
      zeta + 3.8436028638364E-006 * TTt2000 * TTt2000 + 0.000000001 * TTt2000 * TTt2000 * TTt2000;

  double mPrecession[3][3];

  mPrecession[0][0] = -sin(z) * sin(zeta) + cos(z) * cos(theta_p) * cos(zeta);
  mPrecession[1][0] = cos(z) * sin(zeta) + sin(z) * cos(theta_p) * cos(zeta);
  mPrecession[2][0] = sin(theta_p) * cos(zeta);

  mPrecession[0][1] = -sin(z) * cos(zeta) - cos(z) * cos(theta_p) * sin(zeta);
  mPrecession[1][1] = cos(z) * cos(zeta) - sin(z) * cos(theta_p) * sin(zeta);
  mPrecession[2][1] = -sin(theta_p) * sin(zeta);

  mPrecession[0][2] = -cos(z) * sin(theta_p);
  mPrecession[1][2] = -sin(z) * sin(theta_p);
  mPrecession[2][2] = cos(theta_p);

  //////////////////////////////////////////////////////////
  // Calculate Nutation Matrix

  double omega_moon = 2.1824386244 - 33.7570459338 * TTt2000 +
                      3.61428599267159E-005 * TTt2000 * TTt2000 +
                      3.87850944887629E-008 * TTt2000 * TTt2000 * TTt2000;

  double deltaPsi = -0.000083388 * sin(omega_moon);
  double deltaEpsilon = 4.46174030725106E-005 * cos(omega_moon);

  double epsilon = 0.4090928042 - 0.0002269655 * TTt2000 -
                   2.86040071854626E-009 * TTt2000 * TTt2000 +
                   8.78967203851589E-009 * TTt2000 * TTt2000 * TTt2000;

  double mNutation[3][3];

  mNutation[0][0] = cos(deltaPsi);
  mNutation[1][0] = cos(epsilon + deltaEpsilon) * sin(deltaPsi);
  mNutation[2][0] = sin(epsilon + deltaEpsilon) * sin(deltaPsi);

  mNutation[0][1] = -cos(epsilon) * sin(deltaPsi);
  mNutation[1][1] = cos(epsilon) * cos(epsilon + deltaEpsilon) * cos(deltaPsi) +
                    sin(epsilon) * sin(epsilon + deltaEpsilon);
  mNutation[2][1] = cos(epsilon) * sin(epsilon + deltaEpsilon) * cos(deltaPsi) -
                    sin(epsilon) * cos(epsilon + deltaEpsilon);

  mNutation[0][2] = -sin(epsilon) * sin(deltaPsi);
  mNutation[1][2] = sin(epsilon) * cos(epsilon + deltaEpsilon) * cos(deltaPsi) -
                    cos(epsilon) * sin(epsilon + deltaEpsilon);
  mNutation[2][2] = sin(epsilon) * sin(epsilon + deltaEpsilon) * cos(deltaPsi) +
                    cos(epsilon) * cos(epsilon + deltaEpsilon);

  //////////////////////////////////////////////////////////
  // Calculate Earth rotation matrix
  // calculate theta

  double mTheta[3][3];
  double Ttemp[3][3];
  getEarthRotationMatrix(timeUTC, mTheta);

  // polar motion is neglected
  MatrixOperations<double>::multiply(mNutation[0], mPrecession[0], Ttemp[0], 3, 3, 3);

  MatrixOperations<double>::multiply(mTheta[0], Ttemp[0], Tfi[0], 3, 3, 3);
};
