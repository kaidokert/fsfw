#ifndef FSFW_HAL_STM32H7_DEFINITIONS_H_
#define FSFW_HAL_STM32H7_DEFINITIONS_H_

#include <utility>

#include "stm32h7xx.h"

namespace stm32h7 {

/**
 * Typedef for STM32 GPIO pair where the first entry is the port used (e.g. GPIOA)
 * and the second entry is the pin number
 */
struct GpioCfg {
  GpioCfg() : port(nullptr), pin(0), altFnc(0){};

  GpioCfg(GPIO_TypeDef* port, uint16_t pin, uint8_t altFnc = 0)
      : port(port), pin(pin), altFnc(altFnc){};
  GPIO_TypeDef* port;
  uint16_t pin;
  uint8_t altFnc;
};

}  // namespace stm32h7

#endif /* #ifndef FSFW_HAL_STM32H7_DEFINITIONS_H_ */
