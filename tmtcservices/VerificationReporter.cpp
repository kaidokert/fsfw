/*
 * VerificationReporter.cpp
 *
 *  Created on: 20.07.2012
 *      Author: baetz
 */

#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tmtcservices/AcceptsVerifyMessageIF.h>
#include <framework/tmtcservices/PusVerificationReport.h>
#include <framework/tmtcservices/VerificationReporter.h>

VerificationReporter::VerificationReporter() :
		acknowledge_queue() {
}

VerificationReporter::~VerificationReporter() {
	//Default, empty
}

void VerificationReporter::sendSuccessReport(uint8_t set_report_id,
		TcPacketBase* current_packet, uint8_t set_step) {
	if (this->acknowledge_queue.getDefaultDestination() == 0) {
		this->initialize();
	}
	PusVerificationMessage message(set_report_id, current_packet->getAcknowledgeFlags(), current_packet->getPacketId(), current_packet->getPacketSequenceControl(), set_step);
	ReturnValue_t status = this->acknowledge_queue.sendToDefault(&message);
	if (status != OSAL::RETURN_OK) {
		error
				<< "VerificationReporter::sendSuccessReport: Error writing to queue. Code: "
				<< (uint16_t) status << std::endl;
	}
}

void VerificationReporter::sendSuccessReport(uint8_t set_report_id,
		uint8_t ackFlags, uint16_t tcPacketId, uint16_t tcSequenceControl, uint8_t set_step) {
	if (this->acknowledge_queue.getDefaultDestination() == 0) {
		this->initialize();
	}
	PusVerificationMessage message(set_report_id, ackFlags, tcPacketId, tcSequenceControl, set_step);
	ReturnValue_t status = this->acknowledge_queue.sendToDefault(&message);
	if (status != OSAL::RETURN_OK) {
		error
				<< "VerificationReporter::sendSuccessReport: Error writing to queue. Code: "
				<< (uint16_t) status << std::endl;
	}
}

void VerificationReporter::sendFailureReport(uint8_t report_id,
		TcPacketBase* current_packet, ReturnValue_t error_code, uint8_t step,
		uint32_t parameter1, uint32_t parameter2) {
	if (this->acknowledge_queue.getDefaultDestination() == 0) {
		this->initialize();
	}
	PusVerificationMessage message(report_id, current_packet->getAcknowledgeFlags(), current_packet->getPacketId(), current_packet->getPacketSequenceControl(), error_code, step,
			parameter1, parameter2);
	ReturnValue_t status = this->acknowledge_queue.sendToDefault(&message);
	if (status != OSAL::RETURN_OK) {
		error
				<< "VerificationReporter::sendFailureReport Error writing to queue. Code: "
				<< (uint16_t) status << std::endl;
	}
}

void VerificationReporter::sendFailureReport(uint8_t report_id,
		uint8_t ackFlags, uint16_t tcPacketId, uint16_t tcSequenceControl, ReturnValue_t error_code, uint8_t step,
		uint32_t parameter1, uint32_t parameter2) {
	if (this->acknowledge_queue.getDefaultDestination() == 0) {
		this->initialize();
	}
	PusVerificationMessage message(report_id, ackFlags, tcPacketId, tcSequenceControl, error_code, step,
			parameter1, parameter2);
	ReturnValue_t status = this->acknowledge_queue.sendToDefault(&message);
	if (status != OSAL::RETURN_OK) {
		error
				<< "VerificationReporter::sendFailureReport Error writing to queue. Code: "
				<< (uint16_t) status << std::endl;
	}
}

void VerificationReporter::initialize() {
	AcceptsVerifyMessageIF* temp = objectManager->get<AcceptsVerifyMessageIF>(
			objects::PUS_VERIFICATION_SERVICE);
	if (temp != NULL) {
		this->acknowledge_queue.setDefaultDestination(
				temp->getVerificationQueue());
	} else {
		error
				<< "VerificationReporter::VerificationReporter: Configuration error."
				<< std::endl;
	}
}
