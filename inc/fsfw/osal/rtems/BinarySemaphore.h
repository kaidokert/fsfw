#ifndef FSFW_INC_FSFW_OSAL_RTEMS_BINARYSEMAPHORE_H_
#define FSFW_INC_FSFW_OSAL_RTEMS_BINARYSEMAPHORE_H_

#include "fsfw/tasks/SemaphoreIF.h"

class BinarySemaphore: public SemaphoreIF {
public:
    BinarySemaphore();
    virtual ~BinarySemaphore();

    // Interface implementation
    ReturnValue_t acquire(TimeoutType timeoutType =
            TimeoutType::BLOCKING, uint32_t timeoutMs = 0) override;
    ReturnValue_t release() override;
    uint8_t getSemaphoreCounter() const override;
private:
};



#endif /* FSFW_INC_FSFW_OSAL_RTEMS_BINARYSEMAPHORE_H_ */
