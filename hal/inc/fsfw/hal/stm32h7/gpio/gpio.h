#ifndef FSFW_HAL_STM32H7_GPIO_GPIO_H_
#define FSFW_HAL_STM32H7_GPIO_GPIO_H_

#include "stm32h7xx.h"

namespace gpio {

void initializeGpioClock(GPIO_TypeDef* gpioPort);

}

#endif /* FSFW_HAL_STM32H7_GPIO_GPIO_H_ */
