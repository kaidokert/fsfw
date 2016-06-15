/*
 * VerificationReporter.h
 *
 *  Created on: 20.07.2012
 *      Author: baetz
 */

#ifndef VERIFICATIONREPORTER_H_
#define VERIFICATIONREPORTER_H_

#include <framework/ipc/MessageQueueSender.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/tmtcservices/PusVerificationReport.h>

class VerificationReporter {
protected:
	MessageQueueSender acknowledge_queue;
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
};

#endif /* VERIFICATIONREPORTER_H_ */
