#ifndef MATRIXOPERATIONS_H_
#define MATRIXOPERATIONS_H_

#include <stdint.h>

#include <cmath>

template <typename T1, typename T2 = T1, typename T3 = T2>
class MatrixOperations {
 public:
  // do not use with result == matrix1 or matrix2
  static void multiply(const T1 *matrix1, const T2 *matrix2, T3 *result, uint8_t rows1,
                       uint8_t columns1, uint8_t columns2) {
    if ((matrix1 == (T1 *)result) || (matrix2 == (T2 *)result)) {
      // SHOULDDO find an implementation that is tolerant to this
      return;
    }
    for (uint8_t resultColumn = 0; resultColumn < columns2; resultColumn++) {
      for (uint8_t resultRow = 0; resultRow < rows1; resultRow++) {
        result[resultColumn + columns2 * resultRow] = 0;
        for (uint8_t i = 0; i < columns1; i++) {
          result[resultColumn + columns2 * resultRow] +=
              matrix1[i + resultRow * columns1] * matrix2[resultColumn + i * columns2];
        }
      }
    }
  }

  static void transpose(const T1 *matrix, T2 *transposed, uint8_t size) {
    uint8_t row, column;
    transposed[0] = matrix[0];
    for (column = 1; column < size; column++) {
      transposed[column + size * column] = matrix[column + size * column];
      for (row = 0; row < column; row++) {
        T1 temp = matrix[column + size * row];
        transposed[column + size * row] = matrix[row + size * column];
        transposed[row + size * column] = temp;
      }
    }
  }

  // Overload transpose to support non symmetrical matrices
  // do not use with transposed == matrix && columns != rows
  static void transpose(const T1 *matrix, T2 *transposed, uint8_t rows, uint8_t columns) {
    uint8_t row, column;
    transposed[0] = matrix[0];
    if (matrix == transposed && columns == rows) {
      transpose(matrix, transposed, rows);
    } else if (matrix == transposed && columns != rows) {
      // not permitted
      return;
    }
    for (column = 0; column < columns; column++) {
      for (row = 0; row < rows; row++) {
        transposed[row + column * rows] = matrix[column + row * columns];
      }
    }
  }

  static void add(const T1 *matrix1, const T2 *matrix2, T3 *result, uint8_t rows, uint8_t columns) {
    for (uint8_t resultColumn = 0; resultColumn < columns; resultColumn++) {
      for (uint8_t resultRow = 0; resultRow < rows; resultRow++) {
        result[resultColumn + columns * resultRow] = matrix1[resultColumn + columns * resultRow] +
                                                     matrix2[resultColumn + columns * resultRow];
      }
    }
  }

  static void subtract(const T1 *matrix1, const T2 *matrix2, T3 *result, uint8_t rows,
                       uint8_t columns) {
    for (uint8_t resultColumn = 0; resultColumn < columns; resultColumn++) {
      for (uint8_t resultRow = 0; resultRow < rows; resultRow++) {
        result[resultColumn + columns * resultRow] = matrix1[resultColumn + columns * resultRow] -
                                                     matrix2[resultColumn + columns * resultRow];
      }
    }
  }

  static void addScalar(const T1 *matrix1, const T2 scalar, T3 *result, uint8_t rows,
                        uint8_t columns) {
    for (uint8_t resultColumn = 0; resultColumn < columns; resultColumn++) {
      for (uint8_t resultRow = 0; resultRow < rows; resultRow++) {
        result[resultColumn + columns * resultRow] =
            matrix1[resultColumn + columns * resultRow] + scalar;
      }
    }
  }

  static void multiplyScalar(const T1 *matrix1, const T2 scalar, T3 *result, uint8_t rows,
                             uint8_t columns) {
    for (uint8_t resultColumn = 0; resultColumn < columns; resultColumn++) {
      for (uint8_t resultRow = 0; resultRow < rows; resultRow++) {
        result[resultColumn + columns * resultRow] =
            matrix1[resultColumn + columns * resultRow] * scalar;
      }
    }
  }
};

#endif /* MATRIXOPERATIONS_H_ */
