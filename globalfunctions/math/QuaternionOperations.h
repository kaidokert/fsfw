#ifndef QUATERNIONOPERATIONS_H_
#define QUATERNIONOPERATIONS_H_

#include <stdint.h>

class QuaternionOperations {
public:
	virtual ~QuaternionOperations();

	static void multiply(const double *q1, const double *q2, double *q);

	static void fromDcm(const double dcm[][3],double *quaternion, uint8_t *index = 0);
	
	static void toDcm(const double *quaternion, double dcm[][3]);

	static void toDcm(const double *quaternion, float dcm[][3]);

	static float norm(const double *quaternion);

	static void normalize(double *quaternion);

	static void normalize(const double *quaternion, double *unitQuaternion);

	static void inverse(const double *quaternion, double *inverseQuaternion);

	/**
	 * returns angle in ]-Pi;Pi] or [0;Pi] if abs == true
	 */
	static double getAngle(const double *quaternion, bool abs = false);

private:
	QuaternionOperations();
};

#endif /* QUATERNIONOPERATIONS_H_ */
