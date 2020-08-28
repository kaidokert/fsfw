#ifndef CONTROLLERSET_H_
#define CONTROLLERSET_H_

#include "DataSet.h"

class ControllerSet :public DataSet {
public:
	ControllerSet();
	virtual ~ControllerSet();

	virtual void setToDefault() = 0;
	void setInvalid();
};

#endif /* CONTROLLERSET_H_ */
