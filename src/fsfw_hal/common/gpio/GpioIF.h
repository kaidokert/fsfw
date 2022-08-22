#ifndef COMMON_GPIO_GPIOIF_H_
#define COMMON_GPIO_GPIOIF_H_

#include <fsfw/devicehandlers/CookieIF.h>
#include <fsfw/returnvalues/returnvalue.h>

#include "gpioDefinitions.h"

class GpioCookie;

/**
 * @brief	This class defines the interface for objects requiring the control
 * 			over GPIOs.
 * @author	J. Meier
 */
class GpioIF {
 public:
  virtual ~GpioIF(){};

  /**
   * @brief   Called by the GPIO using object.
   * @param cookie	Cookie specifying informations of the GPIOs required
   * 					by a object.
   */
  virtual ReturnValue_t addGpios(GpioCookie* cookie) = 0;

  /**
   * @brief	By implementing this function a child must provide the
   * 			functionality to pull a certain GPIO to high logic level.
   *
   * @param gpioId	A unique number which specifies the GPIO to drive.
   * @return  Returns returnvalue::OK for success. This should never return returnvalue::FAILED.
   */
  virtual ReturnValue_t pullHigh(gpioId_t gpioId) = 0;

  /**
   * @brief	By implementing this function a child must provide the
   * 			functionality to pull a certain GPIO to low logic level.
   *
   * @param gpioId	A unique number which specifies the GPIO to drive.
   */
  virtual ReturnValue_t pullLow(gpioId_t gpioId) = 0;

  /**
   * @brief   This function requires a child to implement the functionality to read the state of
   *          an ouput or input gpio.
   *
   * @param gpioId    A unique number which specifies the GPIO to read.
   * @param gpioState State of GPIO will be written to this pointer.
   */
  virtual ReturnValue_t readGpio(gpioId_t gpioId, int* gpioState) = 0;
};

#endif /* COMMON_GPIO_GPIOIF_H_ */
