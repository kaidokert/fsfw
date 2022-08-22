#ifndef BSP_RPI_GPIO_GPIORPI_H_
#define BSP_RPI_GPIO_GPIORPI_H_

#include <fsfw/returnvalues/returnvalue.h>

#include "../../common/gpio/gpioDefinitions.h"

class GpioCookie;

namespace gpio {

/**
 * Create a GpioConfig_t. This function does a sanity check on the BCM pin number and fails if the
 * BCM pin is invalid.
 * @param cookie    Adds the configuration to this cookie directly
 * @param gpioId    ID which identifies the GPIO configuration
 * @param bcmPin    Raspberry Pi BCM pin
 * @param consumer  Information string
 * @param direction GPIO direction
 * @param initValue Intial value for output pins, 0 for low, 1 for high
 * @return
 */
ReturnValue_t createRpiGpioConfig(GpioCookie* cookie, gpioId_t gpioId, int bcmPin,
                                  std::string consumer, gpio::Direction direction,
                                  gpio::Levels initValue);
}  // namespace gpio

#endif /* BSP_RPI_GPIO_GPIORPI_H_ */
