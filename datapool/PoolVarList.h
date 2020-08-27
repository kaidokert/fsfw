#ifndef POOLVARLIST_H_
#define POOLVARLIST_H_

#include "PoolVariable.h"
#include "PoolVariableIF.h"
template <class T, uint8_t n_var>
class PoolVarList {
private:
	PoolVariable<T> variables[n_var];
public:
	PoolVarList( const uint32_t set_id[n_var], DataSetIF* dataSet, PoolVariableIF::ReadWriteMode_t setReadWriteMode ) {
		//I really should have a look at the new init list c++ syntax.
		if (dataSet == NULL) {
			return;
		}
		for (uint8_t count = 0; count < n_var; count++) {
			variables[count].dataPoolId = set_id[count];
			variables[count].readWriteMode = setReadWriteMode;
			dataSet->registerVariable(&variables[count]);
		}
	}

	PoolVariable<T> &operator [](int i) { return variables[i]; }
};



#endif /* POOLVARLIST_H_ */
