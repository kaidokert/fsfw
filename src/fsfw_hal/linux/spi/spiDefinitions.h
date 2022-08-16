#ifndef LINUX_SPI_SPIDEFINITONS_H_
#define LINUX_SPI_SPIDEFINITONS_H_

#include <linux/spi/spidev.h>

#include <cstdint>

#include "../../common/gpio/gpioDefinitions.h"
#include "../../common/spi/spiCommon.h"
#include "fsfw/returnvalues/returnvalue.h"

class SpiCookie;
class SpiComIF;

namespace spi {

enum SpiComIfModes { REGULAR, CALLBACK };

using send_callback_function_t = ReturnValue_t (*)(SpiComIF* comIf, SpiCookie* cookie,
                                                   const uint8_t* sendData, size_t sendLen,
                                                   void* args);

}  // namespace spi

#endif /* LINUX_SPI_SPIDEFINITONS_H_ */
