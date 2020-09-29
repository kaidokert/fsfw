#ifndef FSFW_PUS_SERVICE9TIMEMANAGEMENT_H_
#define FSFW_PUS_SERVICE9TIMEMANAGEMENT_H_

#include "../tmtcservices/PusServiceBase.h"

class Service9TimeManagement: public PusServiceBase {
public:

	static constexpr uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::PUS_SERVICE_9;
	static constexpr Event CLOCK_SET = MAKE_EVENT(0, SEVERITY::INFO); //!< Clock has been set. P1: New Uptime. P2: Old Uptime
	static constexpr Event CLOCK_SET_FAILURE = MAKE_EVENT(1, SEVERITY::LOW); //!< Clock could not be set. P1: Returncode.

	static constexpr uint8_t CLASS_ID = CLASS_ID::PUS_SERVICE_9;

	/**
	 * 	@brief This service provides the capability to set the on-board time.
	 */
	Service9TimeManagement(object_id_t objectId, uint16_t apid,
			uint8_t serviceId);

	virtual ~Service9TimeManagement();

	virtual ReturnValue_t performService() override;

	/**
	 * 	@brief Sets the onboard-time by retrieving the time to set from TC[9,128].
	 */
	virtual ReturnValue_t handleRequest(uint8_t subservice) override;

	virtual ReturnValue_t setTime();
private:

	enum SUBSERVICE {
		SET_TIME = 128 //!< [EXPORT] : [COMMAND] Time command in ASCII, CUC or CDS format
	};

};



#endif /* FSFW_PUS_SERVICE9TIMEMANAGEMENT_H_ */
