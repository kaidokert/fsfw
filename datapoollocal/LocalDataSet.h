#ifndef FSFW_DATAPOOLLOCAL_LOCALDATASET_H_
#define FSFW_DATAPOOLLOCAL_LOCALDATASET_H_

#include "LocalPoolDataSetBase.h"
#include <vector>

class LocalDataSet: public LocalPoolDataSetBase {
public:
    LocalDataSet(HasLocalDataPoolIF* hkOwner, uint32_t setId,
    		const size_t maxSize);
    LocalDataSet(sid_t sid, const size_t maxSize);
    virtual~ LocalDataSet();

    //! Copying forbidden for now.
    LocalDataSet(const LocalDataSet&) = delete;
    LocalDataSet& operator=(const LocalDataSet&) = delete;
private:
    std::vector<PoolVariableIF*> poolVarList;
};

#endif /* FSFW_DATAPOOLLOCAL_LOCALDATASET_H_ */
