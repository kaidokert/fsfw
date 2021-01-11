#ifndef FSFW_DATAPOOLLOCAL_ACCESSLOCALDATAPOOLIF_H_
#define FSFW_DATAPOOLLOCAL_ACCESSLOCALDATAPOOLIF_H_

#include <fsfw/datapoollocal/LocalDataPoolManager.h>


class AccessLocalDataPoolIF {
public:
	virtual ~AccessLocalDataPoolIF() {};

protected:
	virtual LocalDataPoolManager* getHkManagerHandle() = 0;

};


#endif /* FSFW_DATAPOOLLOCAL_ACCESSLOCALDATAPOOLIF_H_ */
