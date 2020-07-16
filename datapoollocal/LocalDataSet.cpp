#include <framework/datapoollocal/LocalDataSet.h>

LocalDataSet::LocalDataSet(HasLocalDataPoolIF *hkOwner, const size_t maxSize):
        LocalDataSetBase(hkOwner, nullptr, maxSize), poolVarList(maxSize) {
    this->setContainer(poolVarList.data());
}

LocalDataSet::LocalDataSet(object_id_t owner, const size_t maxSize):
        LocalDataSetBase(owner, nullptr, maxSize), poolVarList(maxSize) {
    this->setContainer(poolVarList.data());
}
