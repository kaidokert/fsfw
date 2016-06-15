#ifndef COORDINATETRANSFORMATIONS_H_
#define COORDINATETRANSFORMATIONS_H_

#include <framework/osal/OSAL.h>

class CoordinateTransformations {
public:
	CoordinateTransformations(uint8_t utcGpsOffset);

	virtual ~CoordinateTransformations();

	void positionEcfToEci(const double* ecfCoordinates, double* eciCoordinates);

	void velocityEcfToEci(const double* ecfVelocity,
			const double* ecfPosition,
			double* eciVelocity);

	double getEarthRotationAngle(timeval time);

	void getEarthRotationMatrix(timeval time, double matrix[][3]);
	void setUtcGpsOffset(uint8_t offset);
private:
	uint8_t utcGpsOffset;
	void ecfToEci(const double* ecfCoordinates, double* eciCoordinates,
			const double* ecfPositionIfCoordinatesAreVelocity);

};

#endif /* COORDINATETRANSFORMATIONS_H_ */
