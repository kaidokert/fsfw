#ifndef FRAMEWORK_COORDINATES_JGM3MODEL_H_
#define FRAMEWORK_COORDINATES_JGM3MODEL_H_

#include <memory.h>

#include <cstdint>

#include "CoordinateTransformations.h"
#include "coordinatesConf.h"
#include "fsfw/globalfunctions/constants.h"
#include "fsfw/globalfunctions/math/VectorOperations.h"
#include "fsfw/globalfunctions/timevalOperations.h"

template <uint8_t DEGREE, uint8_t ORDER>
class Jgm3Model {
 public:
  static const uint32_t
      factorialLookupTable[DEGREE + 3];  // This table is used instead of factorial calculation,
                                         // must be increased if order or degree is higher

  Jgm3Model() {
    y0[0] = 0;
    y0[1] = 0;
    y0[2] = 0;
    y0[3] = 0;
    y0[4] = 0;
    y0[5] = 0;

    lastExecutionTime.tv_sec = 0;
    lastExecutionTime.tv_usec = 0;
  }
  virtual ~Jgm3Model(){};

  // double acsNavOrbit(double posECF[3],double velECF[3],timeval gpsTime);

  double y0[6];               // position and velocity at beginning of RK step in EC
  timeval lastExecutionTime;  // Time of last execution

  void accelDegOrd(const double pos[3], const double S[ORDER + 1][DEGREE + 1],
                   const double C[ORDER + 1][DEGREE + 1], double* accel) {
    // Get radius of this position
    double r = VectorOperations<double>::norm(pos, 3);

    // Initialize the V and W matrix
    double V[DEGREE + 2][ORDER + 2] = {{0}};
    double W[DEGREE + 2][ORDER + 2] = {{0}};

    for (uint8_t m = 0; m < (ORDER + 2); m++) {
      for (uint8_t n = m; n < (DEGREE + 2); n++) {
        if ((n == 0) && (m == 0)) {
          // Montenbruck "Satellite Orbits Eq.3.31"
          V[0][0] = Earth::MEAN_RADIUS / r;
          W[0][0] = 0;
        } else {
          if (n == m) {
            // Montenbruck "Satellite Orbits Eq.3.29"
            V[m][m] = (2 * m - 1) * (pos[0] * Earth::MEAN_RADIUS / pow(r, 2) * V[m - 1][m - 1] -
                                     pos[1] * Earth::MEAN_RADIUS / pow(r, 2) * W[m - 1][m - 1]);
            W[m][m] = (2 * m - 1) * (pos[0] * Earth::MEAN_RADIUS / pow(r, 2) * W[m - 1][m - 1] +
                                     pos[1] * Earth::MEAN_RADIUS / pow(r, 2) * V[m - 1][m - 1]);
          } else {
            // Montenbruck "Satellite Orbits Eq.3.30"
            V[n][m] = ((2 * n - 1) / (double)(n - m)) * pos[2] * Earth::MEAN_RADIUS / pow(r, 2) *
                      V[n - 1][m];
            W[n][m] = ((2 * n - 1) / (double)(n - m)) * pos[2] * Earth::MEAN_RADIUS / pow(r, 2) *
                      W[n - 1][m];
            if (n != (m + 1)) {
              V[n][m] = V[n][m] - (((n + m - 1) / (double)(n - m)) *
                                   (pow(Earth::MEAN_RADIUS, 2) / pow(r, 2)) * V[n - 2][m]);
              W[n][m] = W[n][m] - (((n + m - 1) / (double)(n - m)) *
                                   (pow(Earth::MEAN_RADIUS, 2) / pow(r, 2)) * W[n - 2][m]);
            }  // End of if(n!=(m+1))
          }    // End of if(n==m){
        }      // End of if(n==0 and m==0)
      }        // End of for(uint8_t n=0;n<(DEGREE+1);n++)
    }          // End of for(uint8_t m=0;m<(ORDER+1);m++)

    // overwrite accel if not properly initialized
    accel[0] = 0;
    accel[1] = 0;
    accel[2] = 0;

    for (uint8_t m = 0; m < (ORDER + 1); m++) {
      for (uint8_t n = m; n < (DEGREE + 1); n++) {
        // Use table lookup to get factorial
        double partAccel[3] = {0};
        double factor = Earth::STANDARD_GRAVITATIONAL_PARAMETER / pow(Earth::MEAN_RADIUS, 2);
        if (m == 0) {
          // Montenbruck "Satellite Orbits Eq.3.33"
          partAccel[0] = factor * (-C[n][0] * V[n + 1][1]);
          partAccel[1] = factor * (-C[n][0] * W[n + 1][1]);
        } else {
          double factMN = static_cast<double>(factorialLookupTable[n - m + 2]) /
                          static_cast<double>(factorialLookupTable[n - m]);
          partAccel[0] = factor * 0.5 *
                         ((-C[n][m] * V[n + 1][m + 1] - S[n][m] * W[n + 1][m + 1]) +
                          factMN * (C[n][m] * V[n + 1][m - 1] + S[n][m] * W[n + 1][m - 1]));
          partAccel[1] = factor * 0.5 *
                         ((-C[n][m] * W[n + 1][m + 1] + S[n][m] * V[n + 1][m + 1]) +
                          factMN * (-C[n][m] * W[n + 1][m - 1] + S[n][m] * V[n + 1][m - 1]));
        }

        partAccel[2] = factor * ((n - m + 1) * (-C[n][m] * V[n + 1][m] - S[n][m] * W[n + 1][m]));

        accel[0] += partAccel[0];
        accel[1] += partAccel[1];
        accel[2] += partAccel[2];
      }  // End of for(uint8_t n=0;n<DEGREE;n++)
    }    // End of uint8_t m=0;m<ORDER;m++
  }

  void initializeNavOrbit(const double position[3], const double velocity[3], timeval timeUTC) {
    CoordinateTransformations::positionEcfToEci(position, &y0[0], &timeUTC);
    CoordinateTransformations::velocityEcfToEci(velocity, position, &y0[3], &timeUTC);
    lastExecutionTime = timeUTC;
  }

  void acsNavOrbit(timeval timeUTC, const double S[ORDER + 1][DEGREE + 1],
                   const double C[ORDER + 1][DEGREE + 1], double outputPos[3],
                   double outputVel[3]) {
    // RK4 Integration for this timestamp
    double deltaT = timevalOperations::toDouble(timeUTC - lastExecutionTime);

    double y0dot[6] = {0, 0, 0, 0, 0, 0};
    double yA[6] = {0, 0, 0, 0, 0, 0};
    double yAdot[6] = {0, 0, 0, 0, 0, 0};
    double yB[6] = {0, 0, 0, 0, 0, 0};
    double yBdot[6] = {0, 0, 0, 0, 0, 0};
    double yC[6] = {0, 0, 0, 0, 0, 0};
    double yCdot[6] = {0, 0, 0, 0, 0, 0};

    // Step One
    rungeKuttaStep(y0, y0dot, lastExecutionTime, S, C);

    // Step Two
    VectorOperations<double>::mulScalar(y0dot, deltaT / 2, yA, 6);
    VectorOperations<double>::add(y0, yA, yA, 6);
    rungeKuttaStep(yA, yAdot, lastExecutionTime, S, C);

    // Step Three
    VectorOperations<double>::mulScalar(yAdot, deltaT / 2, yB, 6);
    VectorOperations<double>::add(y0, yB, yB, 6);
    rungeKuttaStep(yB, yBdot, lastExecutionTime, S, C);

    // Step Four
    VectorOperations<double>::mulScalar(yBdot, deltaT, yC, 6);
    VectorOperations<double>::add(y0, yC, yC, 6);
    rungeKuttaStep(yC, yCdot, lastExecutionTime, S, C);

    // Calc new State
    VectorOperations<double>::mulScalar(yAdot, 2, yAdot, 6);
    VectorOperations<double>::mulScalar(yBdot, 2, yBdot, 6);
    VectorOperations<double>::add(y0dot, yAdot, y0dot, 6);
    VectorOperations<double>::add(y0dot, yBdot, y0dot, 6);
    VectorOperations<double>::add(y0dot, yCdot, y0dot, 6);
    VectorOperations<double>::mulScalar(y0dot, 1. / 6. * deltaT, y0dot, 6);
    VectorOperations<double>::add(y0, y0dot, y0, 6);

    CoordinateTransformations::positionEciToEcf(&y0[0], outputPos, &timeUTC);
    CoordinateTransformations::velocityEciToEcf(&y0[3], &y0[0], outputVel, &timeUTC);

    lastExecutionTime = timeUTC;
  }

  void rungeKuttaStep(const double* yIn, double* yOut, timeval time,
                      const double S[ORDER + 1][DEGREE + 1],
                      const double C[ORDER + 1][DEGREE + 1]) {
    double rECF[3] = {0, 0, 0};
    double rDotECF[3] = {0, 0, 0};
    double accelECF[3] = {0, 0, 0};
    double accelECI[3] = {0, 0, 0};

    CoordinateTransformations::positionEciToEcf(&yIn[0], rECF, &time);
    CoordinateTransformations::velocityEciToEcf(&yIn[3], &yIn[0], rDotECF, &time);
    accelDegOrd(rECF, S, C, accelECF);
    // This is not correct, as the acceleration would have derived terms but we don't know the
    // velocity and position at that time Tests showed that a wrong velocity does make the equation
    // worse than neglecting it
    CoordinateTransformations::positionEcfToEci(accelECF, accelECI, &time);
    memcpy(&yOut[0], &yIn[3], sizeof(yOut[0]) * 3);
    memcpy(&yOut[3], accelECI, sizeof(yOut[0]) * 3);
  }
};

#endif /* FRAMEWORK_COORDINATES_JGM3MODEL_H_ */
