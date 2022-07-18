#include "fsfw_hal/stm32h7/gpio/gpio.h"

#include "stm32h7xx_hal_rcc.h"

void gpio::initializeGpioClock(GPIO_TypeDef* gpioPort) {
#ifdef GPIOA
  if (gpioPort == GPIOA) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  }
#endif

#ifdef GPIOB
  if (gpioPort == GPIOB) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  }
#endif

#ifdef GPIOC
  if (gpioPort == GPIOC) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  }
#endif

#ifdef GPIOD
  if (gpioPort == GPIOD) {
    __HAL_RCC_GPIOD_CLK_ENABLE();
  }
#endif

#ifdef GPIOE
  if (gpioPort == GPIOE) {
    __HAL_RCC_GPIOE_CLK_ENABLE();
  }
#endif

#ifdef GPIOF
  if (gpioPort == GPIOF) {
    __HAL_RCC_GPIOF_CLK_ENABLE();
  }
#endif

#ifdef GPIOG
  if (gpioPort == GPIOG) {
    __HAL_RCC_GPIOG_CLK_ENABLE();
  }
#endif

#ifdef GPIOH
  if (gpioPort == GPIOH) {
    __HAL_RCC_GPIOH_CLK_ENABLE();
  }
#endif

#ifdef GPIOI
  if (gpioPort == GPIOI) {
    __HAL_RCC_GPIOI_CLK_ENABLE();
  }
#endif

#ifdef GPIOJ
  if (gpioPort == GPIOJ) {
    __HAL_RCC_GPIOJ_CLK_ENABLE();
  }
#endif

#ifdef GPIOK
  if (gpioPort == GPIOK) {
    __HAL_RCC_GPIOK_CLK_ENABLE();
  }
#endif
}
