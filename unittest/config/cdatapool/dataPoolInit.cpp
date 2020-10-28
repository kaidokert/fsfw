#include "dataPoolInit.h"

void datapool::dataPoolInit(std::map<uint32_t, PoolEntryIF*> * poolMap) {
	uint8_t UINT8T_INIT[1] = {0};
	uint16_t UINT16T_INIT[1] = {0};
	uint32_t UINT32T_INIT[1] = {0};
	float FLOAT_INIT[2] = {0.0, 0.0};
	/* FSFW */
	poolMap->emplace(datapool::INTERNAL_ERROR_STORE_FULL,
			new PoolEntry<uint32_t>(UINT32T_INIT,1));
	poolMap->emplace(datapool::INTERNAL_ERROR_MISSED_LIVE_TM,
			new PoolEntry<uint32_t>(UINT32T_INIT,1));
	poolMap->emplace(datapool::INTERNAL_ERROR_FULL_MSG_QUEUES,
			new PoolEntry<uint32_t>(UINT32T_INIT,1));

	/* TEST */
	poolMap->emplace(datapool::TEST_UINT8,
			new PoolEntry<uint8_t>(UINT8T_INIT,1));
	poolMap->emplace(datapool::TEST_UINT16,
			new PoolEntry<uint16_t>(UINT16T_INIT,1));
	poolMap->emplace(datapool::TEST_UINT32,
			new PoolEntry<uint32_t>(UINT32T_INIT,1));
	poolMap->emplace(datapool::TEST_FLOAT_VECTOR,
			new PoolEntry<float>(FLOAT_INIT,2));

	// With new initializer list feature and boolean entries.

//	/* FSFW */
//	poolMap->emplace(datapool::INTERNAL_ERROR_STORE_FULL,
//			new PoolEntry<uint32_t>({0},1));
//	poolMap->emplace(datapool::INTERNAL_ERROR_MISSED_LIVE_TM,
//			new PoolEntry<uint32_t>({0},1));
//	poolMap->emplace(datapool::INTERNAL_ERROR_FULL_MSG_QUEUES,
//			new PoolEntry<uint32_t>({0},1));
//
//	/* TEST */
//	poolMap->emplace(datapool::TEST_BOOLEAN,
//			new PoolEntry<bool>({0},1));
//	poolMap->emplace(datapool::TEST_UINT8,
//			new PoolEntry<uint8_t>({0},1));
//	poolMap->emplace(datapool::TEST_UINT16,
//			new PoolEntry<uint16_t>({0},1));
//	poolMap->emplace(datapool::TEST_UINT32,
//			new PoolEntry<uint32_t>({0},1));
//	poolMap->emplace(datapool::TEST_FLOAT_VECTOR,
//			new PoolEntry<float>({0, 0},2));

}
