#ifndef FSFW_DATAPOOLLOCAL_STATICLOCALDATASET_H_
#define FSFW_DATAPOOLLOCAL_STATICLOCALDATASET_H_

#include "LocalPoolDataSetBase.h"
#include "../objectmanager/SystemObjectIF.h"
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
class StaticLocalDataSet: public LocalPoolDataSetBase {
public:
	/**
	 * Constructor used by data owner and creator like device handlers.
	 * This constructor also initialized the components required for
	 * periodic handling.
	 * @param hkOwner
	 * @param setId
	 */
    StaticLocalDataSet(HasLocalDataPoolIF* hkOwner,
    		uint32_t setId): LocalPoolDataSetBase(hkOwner, setId, nullptr,
            NUM_VARIABLES) {
        this->setContainer(poolVarList.data());
    }

	/**
	 * Constructor used by data users like controllers.
	 * @param hkOwner
	 * @param setId
	 */
    StaticLocalDataSet(sid_t sid): LocalPoolDataSetBase(sid, nullptr,
            NUM_VARIABLES) {
        this->setContainer(poolVarList.data());
    }

private:
    std::array<PoolVariableIF*, NUM_VARIABLES> poolVarList;
};

#endif /* FSFW_DATAPOOLLOCAL_STATICLOCALDATASET_H_ */
