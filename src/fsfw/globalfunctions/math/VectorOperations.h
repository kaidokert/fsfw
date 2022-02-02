#ifndef VECTOROPERATIONS_
#define VECTOROPERATIONS_

#include <stdint.h>

#include <cmath>

template <typename T>
class VectorOperations {
 public:
  virtual ~VectorOperations() {}

  static void cross(const T left[], const T right[], T out[]) {
    T temp[3] = {0, 0, 0};
    temp[0] = left[1] * right[2] - left[2] * right[1];
    temp[1] = left[2] * right[0] - left[0] * right[2];
    temp[2] = left[0] * right[1] - left[1] * right[0];
    out[0] = temp[0];
    out[1] = temp[1];
    out[2] = temp[2];
  }

  static T dot(const T a[], const T b[]) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }

  static void mulScalar(const T vector[], T scalar, T out[], uint8_t size) {
    for (; size > 0; size--) {
      out[size - 1] = vector[size - 1] * scalar;
    }
  }

  static void add(const T vector1[], const T vector2[], T sum[], uint8_t size = 3) {
    for (; size > 0; size--) {
      sum[size - 1] = vector1[size - 1] + vector2[size - 1];
    }
  }

  static void subtract(const T vector1[], const T vector2[], T sum[], uint8_t size = 3) {
    for (; size > 0; size--) {
      sum[size - 1] = vector1[size - 1] - vector2[size - 1];
    }
  }

  static T norm(const T *vector, uint8_t size) {
    T result = 0;
    for (; size > 0; size--) {
      result += vector[size - 1] * vector[size - 1];
    }
    result = sqrt(result);
    return result;
  }

  static void normalize(const T *vector, T *normalizedVector, uint8_t size) {
    mulScalar(vector, 1 / norm(vector, size), normalizedVector, size);
  }

  static T maxAbsValue(const T *vector, uint8_t size, uint8_t *index = 0) {
    T max = -1;

    for (; size > 0; size--) {
      T abs = vector[size - 1];
      if (abs < 0) {
        abs = -abs;
      }
      if (abs > max) {
        max = abs;
        if (index != 0) {
          *index = size - 1;
        }
      }
    }
    return max;
  }

  static T maxValue(const T *vector, uint8_t size, uint8_t *index = 0) {
    T max = -1;

    for (; size > 0; size--) {
      if (vector[size - 1] > max) {
        max = vector[size - 1];
        if (index != 0) {
          *index = size - 1;
        }
      }
    }
    return max;
  }

  static void copy(const T *in, T *out, uint8_t size) { mulScalar(in, 1, out, size); }

 private:
  VectorOperations();
};

#endif /* VECTOROPERATIONS_ */
