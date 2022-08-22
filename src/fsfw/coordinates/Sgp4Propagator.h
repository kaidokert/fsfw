#ifndef SGP4PROPAGATOR_H_
#define SGP4PROPAGATOR_H_

#include "coordinatesConf.h"
#include "fsfw/platform.h"

#ifndef PLATFORM_WIN
#include <sys/time.h>
#endif
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw_contrib/sgp4/sgp4unit.h"

class Sgp4Propagator {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::SGP4PROPAGATOR_CLASS;
  static const ReturnValue_t INVALID_ECCENTRICITY = MAKE_RETURN_CODE(0xA1);
  static const ReturnValue_t INVALID_MEAN_MOTION = MAKE_RETURN_CODE(0xA2);
  static const ReturnValue_t INVALID_PERTURBATION_ELEMENTS = MAKE_RETURN_CODE(0xA3);
  static const ReturnValue_t INVALID_SEMI_LATUS_RECTUM = MAKE_RETURN_CODE(0xA4);
  static const ReturnValue_t INVALID_EPOCH_ELEMENTS = MAKE_RETURN_CODE(0xA5);
  static const ReturnValue_t SATELLITE_HAS_DECAYED = MAKE_RETURN_CODE(0xA6);
  static const ReturnValue_t TLE_TOO_OLD = MAKE_RETURN_CODE(0xB1);
  static const ReturnValue_t TLE_NOT_INITIALIZED = MAKE_RETURN_CODE(0xB2);

  Sgp4Propagator();
  virtual ~Sgp4Propagator();

  ReturnValue_t initialize(const uint8_t *line1, const uint8_t *line2);

  /**
   *
   * @param[out] position in ECF
   * @param[out] velocity in ECF
   * @param time to which to propagate
   * @return
   */
  ReturnValue_t propagate(double *position, double *velocity, timeval time, uint8_t gpsUtcOffset);

 private:
  bool initialized;
  timeval epoch;
  elsetrec satrec;
  gravconsttype whichconst;
};

#endif /* SGP4PROPAGATOR_H_ */
