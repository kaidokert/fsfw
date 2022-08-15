#include "fsfw_hal/linux/rpi/GpioRPi.h"

#include <fsfw/serviceinterface/ServiceInterface.h>

#include "fsfw/FSFW.h"
#include "fsfw_hal/common/gpio/GpioCookie.h"

ReturnValue_t gpio::createRpiGpioConfig(GpioCookie* cookie, gpioId_t gpioId, int bcmPin,
                                        std::string consumer, gpio::Direction direction,
                                        gpio::Levels initValue) {
  if (cookie == nullptr) {
    return returnvalue::FAILED;
  }

  auto config = new GpiodRegularByChip();
  /* Default chipname for Raspberry Pi. There is still gpiochip1 for expansion, but most users
  will not need this */
  config->chipname = "gpiochip0";

  config->consumer = consumer;
  config->direction = direction;
  config->initValue = initValue;

  /* Sanity check for the BCM pins before assigning it */
  if (bcmPin > 27) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "createRpiGpioConfig: BCM pin " << bcmPin << " invalid!" << std::endl;
#else
    sif::printError("createRpiGpioConfig: BCM pin %d invalid!\n", bcmPin);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    return returnvalue::FAILED;
  }
  config->lineNum = bcmPin;
  cookie->addGpio(gpioId, config);
  return returnvalue::OK;
}
