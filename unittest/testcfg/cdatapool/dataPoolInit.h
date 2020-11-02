#ifndef HOSTED_CONFIG_CDATAPOOL_DATAPOOLINIT_H_
#define HOSTED_CONFIG_CDATAPOOL_DATAPOOLINIT_H_

#include <fsfw/datapool/DataPool.h>
#include <fsfw/datapool/PoolEntryIF.h>
#include <map>
#include <cstdint>


namespace datapool {
	void dataPoolInit(std::map<uint32_t, PoolEntryIF*> * poolMap);

	enum datapoolvariables {
		NO_PARAMETER = 0,
	};
}
#endif /* CONFIG_CDATAPOOL_DATAPOOLINIT_H_ */
