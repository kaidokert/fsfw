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

		/** [EXPORT] : [GROUP] FSFW */
		INTERNAL_ERROR_STORE_FULL = 0xEE000001, 	 //!< [EXPORT] : [NAME] Internal Error Store Entry [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		INTERNAL_ERROR_MISSED_LIVE_TM = 0xEE000001,  //!< [EXPORT] : [NAME] Internal Error Missed Live Tm [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		INTERNAL_ERROR_FULL_MSG_QUEUES = 0xEE000001, //!< [EXPORT] : [NAME] Internal Error Full Msg Queue [UNIT] (-) [SIZE] 1 [TYPE] uint32_t

		/** [EXPORT] : [GROUP] TEST */
		TEST_BOOLEAN = 0x01010102, 				//!< [EXPORT] : [NAME] Test Boolean [UNIT] (-) [SIZE] 1 [TYPE] bool
		TEST_UINT8 = 0x02020204,				//!< [EXPORT] : [NAME] Test Byte [UNIT] (-) [SIZE] 1 [TYPE] uint8_t
		TEST_UINT16 = 0x03030306,				//!< [EXPORT] : [NAME] Test UINT16 [UNIT] (-) [SIZE] 1 [TYPE] uint16_t
		TEST_UINT32 = 0x04040408,				//!< [EXPORT] : [NAME] Test UINT32 [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		TEST_FLOAT_VECTOR = 0x05050510,			//!< [EXPORT] : [NAME] Test Float [UNIT] (-) [SIZE] 2 [TYPE] float
	};
}
#endif /* CONFIG_CDATAPOOL_DATAPOOLINIT_H_ */
