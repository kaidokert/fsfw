#ifndef FRAMEWORK_DATAPOOLLOCAL_STATICLOCALDATASET_H_
#define FRAMEWORK_DATAPOOLLOCAL_STATICLOCALDATASET_H_
#include <framework/datapoollocal/LocalPoolDataSetBase.h>
#include <framework/objectmanager/SystemObjectIF.h>
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
template <uint8_t NUM_VARIABLES>
class StaticLocalDataSet: public LocalDataSetBase {
public:
    StaticLocalDataSet(object_id_t owner):
        LocalDataSetBase(owner, poolVarList.data(), NUM_VARIABLES) {
    }

    virtual~ StaticLocalDataSet() {};
private:
    std::array<PoolVariableIF*, NUM_VARIABLES> poolVarList;
};

#endif /* FRAMEWORK_DATAPOOLLOCAL_STATICLOCALDATASET_H_ */
