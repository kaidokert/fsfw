#ifndef FSFW_DATAPOOLLOCAL_STATICLOCALDATASET_H_
#define FSFW_DATAPOOLLOCAL_STATICLOCALDATASET_H_

#include <array>

#include "../objectmanager/SystemObjectIF.h"
#include "LocalPoolDataSetBase.h"
#include "LocalPoolVariable.h"
#include "LocalPoolVector.h"

/**
 * @brief   This dataset type can be used to group related pool variables if the number of
 *          variables is fixed.
 * @details
 * This will is the primary data structure to organize pool variables into
 * sets which can be accessed via the housekeeping service interface or
 * which can be sent to other software objects.
 *
 * It is recommended to read the documentation of the LocalPoolDataSetBase
 * class for more information on how this class works and how to use it.
 * @tparam capacity Capacity of the static dataset, which is usually known
 * beforehand.
 */
template <uint8_t NUM_VARIABLES>
class StaticLocalDataSet : public LocalPoolDataSetBase {
 public:
  /**
   * Constructor used by data owner and creator like device handlers.
   * This constructor also initialized the components required for
   * periodic handling.
   * @param hkOwner
   * @param setId
   */
  StaticLocalDataSet(HasLocalDataPoolIF* hkOwner, uint32_t setId)
      : LocalPoolDataSetBase(hkOwner, setId, nullptr, NUM_VARIABLES) {
    this->setContainer(poolVarList.data());
  }

  /**
   * Constructor used by data users like controllers.
   * @param hkOwner
   * @param setId
   */
  StaticLocalDataSet(sid_t sid) : LocalPoolDataSetBase(sid, nullptr, NUM_VARIABLES) {
    this->setContainer(poolVarList.data());
  }

 private:
  std::array<PoolVariableIF*, NUM_VARIABLES> poolVarList = {};
};

#endif /* FSFW_DATAPOOLLOCAL_STATICLOCALDATASET_H_ */
