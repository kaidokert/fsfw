#ifndef LINUX_SPI_SPIDEFINITONS_H_
#define LINUX_SPI_SPIDEFINITONS_H_

#include "../../common/gpio/gpioDefinitions.h"
#include "../../common/spi/spiCommon.h"

#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include <linux/spi/spidev.h>

#include <cstdint>

class SpiCookie;
class SpiComIF;

namespace spi {

enum SpiComIfModes {
	REGULAR,
	CALLBACK
};


using send_callback_function_t = ReturnValue_t (*) (SpiComIF* comIf, SpiCookie *cookie,
        const uint8_t *sendData, size_t sendLen, void* args);

}

#endif /* LINUX_SPI_SPIDEFINITONS_H_ */
