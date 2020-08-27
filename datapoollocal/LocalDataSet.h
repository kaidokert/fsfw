#ifndef FRAMEWORK_DATAPOOLLOCAL_LOCALDATASET_H_
#define FRAMEWORK_DATAPOOLLOCAL_LOCALDATASET_H_
#include "../datapoollocal/LocalPoolDataSetBase.h"
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

#endif /* FRAMEWORK_DATAPOOLLOCAL_LOCALDATASET_H_ */
