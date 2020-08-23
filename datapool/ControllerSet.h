#ifndef CONTROLLERSET_H_
#define CONTROLLERSET_H_

#include "../datapoolglob/GlobalDataSet.h"

class ControllerSet :public GlobDataSet {
public:
	ControllerSet();
	virtual ~ControllerSet();

	virtual void setToDefault() = 0;
	void setInvalid();
};

#endif /* CONTROLLERSET_H_ */
