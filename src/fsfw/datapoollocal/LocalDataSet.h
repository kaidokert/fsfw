#ifndef FSFW_DATAPOOLLOCAL_LOCALDATASET_H_
#define FSFW_DATAPOOLLOCAL_LOCALDATASET_H_

#include <vector>

#include "LocalPoolDataSetBase.h"

/**
 * @brief   This dataset type can be used to group related pool variables if the number of
 *          variables should not be fixed.
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
class LocalDataSet : public LocalPoolDataSetBase {
 public:
  LocalDataSet(HasLocalDataPoolIF* hkOwner, uint32_t setId, const size_t maxSize);

  LocalDataSet(sid_t sid, const size_t maxSize);

  virtual ~LocalDataSet();

  //! Copying forbidden for now.
  LocalDataSet(const LocalDataSet&) = delete;
  LocalDataSet& operator=(const LocalDataSet&) = delete;

 private:
  std::vector<PoolVariableIF*> poolVarList;
};

#endif /* FSFW_DATAPOOLLOCAL_LOCALDATASET_H_ */
