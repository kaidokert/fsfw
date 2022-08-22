#ifndef COMMON_GPIO_GPIOCOOKIE_H_
#define COMMON_GPIO_GPIOCOOKIE_H_

#include <fsfw/devicehandlers/CookieIF.h>
#include <fsfw/returnvalues/returnvalue.h>

#include "GpioIF.h"
#include "gpioDefinitions.h"

/**
 * @brief   Cookie for the GpioIF. Allows the GpioIF to determine which
 * 			GPIOs to initialize and whether they should be configured as in- or
 * 			output.
 * @details	One GpioCookie can hold multiple GPIO configurations. To add a new
 * 			GPIO configuration to a GpioCookie use the GpioCookie::addGpio
 * 			function.
 *
 * @author 	J. Meier
 */
class GpioCookie : public CookieIF {
 public:
  GpioCookie();

  virtual ~GpioCookie();

  ReturnValue_t addGpio(gpioId_t gpioId, GpioBase* gpioConfig);

  /**
   * @brief	Get map with registered GPIOs.
   */
  GpioMap getGpioMap() const;

 private:
  /**
   * Returns a copy of the internal GPIO map.
   */
  GpioMap gpioMap;
};

#endif /* COMMON_GPIO_GPIOCOOKIE_H_ */
