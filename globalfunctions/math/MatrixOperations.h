#ifndef MATRIXOPERATIONS_H_
#define MATRIXOPERATIONS_H_

#include <cmath>
#include <stdint.h>

template<typename T>
class MatrixOperations {
public:
	virtual ~MatrixOperations() {
	}

	//do not use with result == matrix1 or matrix2 //TODO?
	static void multiply(const T *matrix1, const T *matrix2, T *result,
			uint8_t rows1, uint8_t columns1, uint8_t columns2) {
		for (uint8_t resultColumn = 0; resultColumn < columns2;
				resultColumn++) {
			for (uint8_t resultRow = 0; resultRow < rows1; resultRow++) {
				result[resultColumn + columns2 * resultRow] = 0;
				for (uint8_t i = 0; i < columns1; i++) {
					result[resultColumn + columns2 * resultRow] += matrix1[i
							+ resultRow * columns1]
							* matrix2[resultColumn + i * columns2];
				}
			}
		}
	}

	static void transpose(const T *matrix, T *transposed, uint8_t size) {
		uint8_t row, column;
		transposed[0] = matrix[0];
		for (column = 1; column < size; column++) {
			transposed[column + size * column] = matrix[column + size * column];
			for (row = 0; row < column; row++) {
				T temp = matrix[column + size * row];
				transposed[column + size * row] = matrix[row + size * column];
				transposed[row + size * column] = temp;
			}
		}
	}

private:
	MatrixOperations();
};

#endif /* MATRIXOPERATIONS_H_ */
