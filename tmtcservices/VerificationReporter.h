#ifndef VERIFICATIONREPORTER_H_
#define VERIFICATIONREPORTER_H_

#include "../objectmanager/ObjectManagerIF.h"
#include "PusVerificationReport.h"

namespace Factory{
void setStaticFrameworkObjectIds();
}

class VerificationReporter {
	friend void (Factory::setStaticFrameworkObjectIds)();
public:
	VerificationReporter();
	virtual ~VerificationReporter();
	void sendSuccessReport( uint8_t set_report_id, TcPacketBase* current_packet, uint8_t set_step = 0 );
	void sendSuccessReport(uint8_t set_report_id, uint8_t ackFlags, uint16_t tcPacketId, uint16_t tcSequenceControl, uint8_t set_step = 0);
	void sendFailureReport( uint8_t report_id, TcPacketBase* current_packet, ReturnValue_t error_code = 0,
			uint8_t step = 0, uint32_t parameter1 = 0, uint32_t parameter2 = 0 );
	void sendFailureReport(uint8_t report_id,
			uint8_t ackFlags, uint16_t tcPacketId, uint16_t tcSequenceControl, ReturnValue_t error_code = 0, uint8_t step = 0,
			uint32_t parameter1 = 0, uint32_t parameter2 = 0);
	void initialize();
private:
	static object_id_t messageReceiver;
	MessageQueueId_t acknowledgeQueue;


};

#endif /* VERIFICATIONREPORTER_H_ */
