#ifndef COORDINATETRANSFORMATIONS_H_
#define COORDINATETRANSFORMATIONS_H_

#include <cstring>

#include "coordinatesConf.h"
#include "fsfw/timemanager/Clock.h"

class CoordinateTransformations {
 public:
  static void positionEcfToEci(const double* ecfCoordinates, double* eciCoordinates,
                               timeval* timeUTC = NULL);

  static void velocityEcfToEci(const double* ecfVelocity, const double* ecfPosition,
                               double* eciVelocity, timeval* timeUTC = NULL);

  static void positionEciToEcf(const double* eciCoordinates, double* ecfCoordinates,
                               timeval* timeUTC = NULL);
  static void velocityEciToEcf(const double* eciVelocity, const double* eciPosition,
                               double* ecfVelocity, timeval* timeUTC = NULL);

  static double getEarthRotationAngle(timeval timeUTC);

  static void getEarthRotationMatrix(timeval timeUTC, double matrix[][3]);

 private:
  CoordinateTransformations();
  static void ecfToEci(const double* ecfCoordinates, double* eciCoordinates,
                       const double* ecfPositionIfCoordinatesAreVelocity, timeval* timeUTCin);
  static void eciToEcf(const double* eciCoordinates, double* ecfCoordinates,
                       const double* eciPositionIfCoordinatesAreVelocity, timeval* timeUTCin);

  static double getJuleanCenturiesTT(timeval timeUTC);
  static void getTransMatrixECITOECF(timeval time, double Tfi[3][3]);
};

#endif /* COORDINATETRANSFORMATIONS_H_ */
