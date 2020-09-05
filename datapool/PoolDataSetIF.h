#ifndef FSFW_DATAPOOL_POOLDATASETIF_H_
#define FSFW_DATAPOOL_POOLDATASETIF_H_

#include "DataSetIF.h"

class PoolDataSetIF: public DataSetIF {
public:
    virtual~ PoolDataSetIF() {};

    virtual ReturnValue_t read(dur_millis_t lockTimeout) = 0;
    virtual ReturnValue_t commit(dur_millis_t lockTimeout) = 0;

    /**
     * @brief   Most underlying data structures will have a pool like structure
     *          and will require a lock and unlock mechanism to ensure
     *          thread-safety
     * @return Lock operation result
     */
    virtual ReturnValue_t lockDataPool(dur_millis_t timeoutMs) = 0;
    /**
     * @brief   Unlock call corresponding to the lock call.
     * @return Unlock operation result
     */
    virtual ReturnValue_t unlockDataPool() = 0;

    virtual bool isValid() const = 0;
};

#endif /* FSFW_DATAPOOL_POOLDATASETIF_H_ */
