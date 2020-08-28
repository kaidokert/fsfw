#ifndef FRAMEWORK_DATAPOOL_PIDREADERLIST_H_
#define FRAMEWORK_DATAPOOL_PIDREADERLIST_H_

#include "PIDReader.h"
#include "PoolVariableIF.h"
template <class T, uint8_t n_var>
class PIDReaderList {
private:
	PIDReader<T> variables[n_var];
public:
	PIDReaderList( const uint32_t setPid[n_var], DataSetIF* dataSet) {
		//I really should have a look at the new init list c++ syntax.
		if (dataSet == NULL) {
			return;
		}
		for (uint8_t count = 0; count < n_var; count++) {
			variables[count].parameterId = setPid[count];
			dataSet->registerVariable(&variables[count]);
		}
	}

	PIDReader<T> &operator [](int i) { return variables[i]; }
};



#endif /* FRAMEWORK_DATAPOOL_PIDREADERLIST_H_ */
