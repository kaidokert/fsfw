#ifndef FSFW_HAL_STM32H7_INTERRUPTS_H_
#define FSFW_HAL_STM32H7_INTERRUPTS_H_

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Default handler which is defined in startup file as assembly code.
 */
extern void Default_Handler();

typedef void (*user_handler_t)(void*);
typedef void* user_args_t;

enum IrqPriorities : uint8_t { HIGHEST = 0, HIGHEST_FREERTOS = 6, LOWEST = 15 };

#ifdef __cplusplus
}
#endif

#endif /* FSFW_HAL_STM32H7_INTERRUPTS_H_ */
