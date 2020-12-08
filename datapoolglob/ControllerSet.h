#ifndef FSFW_DATAPOOLGLOB_CONTROLLERSET_H_
#define FSFW_DATAPOOLGLOB_CONTROLLERSET_H_

#include "../datapoolglob/GlobalDataSet.h"

class ControllerSet :public GlobDataSet {
public:
	ControllerSet();
	virtual ~ControllerSet();

	virtual void setToDefault() = 0;
	void setInvalid();
};

#endif /*  FSFW_DATAPOOLGLOB_CONTROLLERSET_H_ */
