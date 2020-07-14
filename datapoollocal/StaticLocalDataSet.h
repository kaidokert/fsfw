#ifndef FRAMEWORK_DATAPOOLLOCAL_STATICLOCALDATASET_H_
#define FRAMEWORK_DATAPOOLLOCAL_STATICLOCALDATASET_H_
#include <framework/datapool/PoolDataSetBase.h>
#include <array>

/**
 * @brief   This local dataset type is created on the stack.
 * @details
 * Size of data set specified as a constructor argument. It is recommended
 * to use the default LocalDataSet of the dataset is constructed on the heap
 * and the SharedLocalDataSet if it created on the heap and used by multiple
 * other software objects.
 * @tparam capacity
 */
template <uint8_t capacity>
class StaticLocalDataSet: public PoolDataSetBase {
    StaticLocalDataSet():  PoolDataSetBase(poolVarList.data(), capacity) {}
    virtual~ StaticLocalDataSet();
private:
    std::array<PoolVariableIF*, capacity> poolVarList;
};

#endif /* FRAMEWORK_DATAPOOLLOCAL_STATICLOCALDATASET_H_ */
