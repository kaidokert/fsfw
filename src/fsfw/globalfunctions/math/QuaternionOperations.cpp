#include "fsfw/globalfunctions/math/QuaternionOperations.h"

#include <stdint.h>

#include <cmath>
#include <cstring>

#include "fsfw/globalfunctions/math/VectorOperations.h"

QuaternionOperations::~QuaternionOperations() {}

void QuaternionOperations::multiply(const double* q1, const double* q2, double* q) {
  double out[4];

  out[0] = q1[3] * q2[0] + q1[2] * q2[1] - q1[1] * q2[2] + q1[0] * q2[3];
  out[1] = -q1[2] * q2[0] + q1[3] * q2[1] + q1[0] * q2[2] + q1[1] * q2[3];
  out[2] = q1[1] * q2[0] - q1[0] * q2[1] + q1[3] * q2[2] + q1[2] * q2[3];
  out[3] = -q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2] + q1[3] * q2[3];

  memcpy(q, out, 4 * sizeof(*q));
}

void QuaternionOperations::toDcm(const double* quaternion, double dcm[][3]) {
  dcm[0][0] = 2 * (quaternion[0] * quaternion[0] + quaternion[3] * quaternion[3]) - 1;
  dcm[0][1] = 2 * (quaternion[0] * quaternion[1] + quaternion[2] * quaternion[3]);
  dcm[0][2] = 2 * (quaternion[0] * quaternion[2] - quaternion[1] * quaternion[3]);

  dcm[1][0] = 2 * (quaternion[0] * quaternion[1] - quaternion[2] * quaternion[3]);
  dcm[1][1] = 2 * (quaternion[1] * quaternion[1] + quaternion[3] * quaternion[3]) - 1;
  dcm[1][2] = 2 * (quaternion[1] * quaternion[2] + quaternion[0] * quaternion[3]);

  dcm[2][0] = 2 * (quaternion[0] * quaternion[2] + quaternion[1] * quaternion[3]);
  dcm[2][1] = 2 * (quaternion[1] * quaternion[2] - quaternion[0] * quaternion[3]);
  dcm[2][2] = 2 * (quaternion[2] * quaternion[2] + quaternion[3] * quaternion[3]) - 1;
}

void QuaternionOperations::inverse(const double* quaternion, double* inverseQuaternion) {
  memcpy(inverseQuaternion, quaternion, 4 * sizeof(*quaternion));
  VectorOperations<double>::mulScalar(inverseQuaternion, -1, inverseQuaternion, 3);
}

QuaternionOperations::QuaternionOperations() {}

void QuaternionOperations::normalize(const double* quaternion, double* unitQuaternion) {
  VectorOperations<double>::normalize(quaternion, unitQuaternion, 4);
}

float QuaternionOperations::norm(const double* quaternion) {
  return VectorOperations<double>::norm(quaternion, 4);
}

void QuaternionOperations::fromDcm(const double dcm[][3], double* quaternion, uint8_t* index) {
  double a[4];

  a[0] = 1 + dcm[0][0] - dcm[1][1] - dcm[2][2];
  a[1] = 1 - dcm[0][0] + dcm[1][1] - dcm[2][2];
  a[2] = 1 - dcm[0][0] - dcm[1][1] + dcm[2][2];
  a[3] = 1 + dcm[0][0] + dcm[1][1] + dcm[2][2];

  uint8_t maxAIndex = 0;

  VectorOperations<double>::maxValue(a, 4, &maxAIndex);

  if (index != 0) {
    *index = maxAIndex;
  }

  switch (maxAIndex) {
    case 0:
      quaternion[0] = 0.5 * sqrt(a[0]);
      quaternion[1] = (dcm[0][1] + dcm[1][0]) / (2 * sqrt(a[0]));
      quaternion[2] = (dcm[0][2] + dcm[2][0]) / (2 * sqrt(a[0]));
      quaternion[3] = (dcm[1][2] - dcm[2][1]) / (2 * sqrt(a[0]));
      break;
    case 1:
      quaternion[0] = (dcm[0][1] + dcm[1][0]) / (2 * sqrt(a[1]));
      quaternion[1] = 0.5 * sqrt(a[1]);
      quaternion[2] = (dcm[1][2] + dcm[2][1]) / (2 * sqrt(a[1]));
      quaternion[3] = (dcm[2][0] - dcm[0][2]) / (2 * sqrt(a[1]));
      break;
    case 2:
      quaternion[0] = (dcm[0][2] + dcm[2][0]) / (2 * sqrt(a[2]));
      quaternion[1] = (dcm[1][2] + dcm[2][1]) / (2 * sqrt(a[2]));
      quaternion[2] = 0.5 * sqrt(a[2]);
      quaternion[3] = (dcm[0][1] - dcm[1][0]) / (2 * sqrt(a[2]));
      break;
    case 3:
      quaternion[0] = (dcm[1][2] - dcm[2][1]) / (2 * sqrt(a[3]));
      quaternion[1] = (dcm[2][0] - dcm[0][2]) / (2 * sqrt(a[3]));
      quaternion[2] = (dcm[0][1] - dcm[1][0]) / (2 * sqrt(a[3]));
      quaternion[3] = 0.5 * sqrt(a[3]);
      break;
  }
}

void QuaternionOperations::toDcm(const double* quaternion, float dcm[][3]) {
  dcm[0][0] = 2 * (quaternion[0] * quaternion[0] + quaternion[3] * quaternion[3]) - 1;
  dcm[0][1] = 2 * (quaternion[0] * quaternion[1] + quaternion[2] * quaternion[3]);
  dcm[0][2] = 2 * (quaternion[0] * quaternion[2] - quaternion[1] * quaternion[3]);

  dcm[1][0] = 2 * (quaternion[0] * quaternion[1] - quaternion[2] * quaternion[3]);
  dcm[1][1] = 2 * (quaternion[1] * quaternion[1] + quaternion[3] * quaternion[3]) - 1;
  dcm[1][2] = 2 * (quaternion[1] * quaternion[2] + quaternion[0] * quaternion[3]);

  dcm[2][0] = 2 * (quaternion[0] * quaternion[2] + quaternion[1] * quaternion[3]);
  dcm[2][1] = 2 * (quaternion[1] * quaternion[2] - quaternion[0] * quaternion[3]);
  dcm[2][2] = 2 * (quaternion[2] * quaternion[2] + quaternion[3] * quaternion[3]) - 1;
}

void QuaternionOperations::normalize(double* quaternion) { normalize(quaternion, quaternion); }

double QuaternionOperations::getAngle(const double* quaternion, bool abs) {
  if (quaternion[3] >= 0) {
    return 2 * acos(quaternion[3]);
  } else {
    if (abs) {
      return 2 * acos(-quaternion[3]);
    } else {
      return -2 * acos(-quaternion[3]);
    }
  }
}
