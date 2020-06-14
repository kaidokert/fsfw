#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tmtcservices/AcceptsVerifyMessageIF.h>
#include <framework/tmtcservices/PusVerificationReport.h>
#include <framework/tmtcservices/VerificationReporter.h>

object_id_t VerificationReporter::messageReceiver = 0;

VerificationReporter::VerificationReporter() :
		acknowledgeQueue() {
}

VerificationReporter::~VerificationReporter() {
	//Default, empty
}

void VerificationReporter::sendSuccessReport(uint8_t set_report_id,
		TcPacketBase* current_packet, uint8_t set_step) {
	if (this->acknowledgeQueue == 0) {
		this->initialize();
	}
	PusVerificationMessage message(set_report_id,
			current_packet->getAcknowledgeFlags(),
			current_packet->getPacketId(),
			current_packet->getPacketSequenceControl(), 0, set_step);
	ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
	if (status != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "VerificationReporter::sendSuccessReport: Error writing "
				"to queue. Code: " << std::hex << (uint16_t) status << std::endl;
	}
}

void VerificationReporter::sendSuccessReport(uint8_t set_report_id,
		uint8_t ackFlags, uint16_t tcPacketId, uint16_t tcSequenceControl,
		uint8_t set_step) {
	if (this->acknowledgeQueue == 0) {
		this->initialize();
	}
	PusVerificationMessage message(set_report_id, ackFlags, tcPacketId,
			tcSequenceControl, 0, set_step);
	ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
	if (status != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "VerificationReporter::sendSuccessReport: Error writing "
				"to queue. Code: " << std::hex << (uint16_t) status << std::endl;
	}
}

void VerificationReporter::sendFailureReport(uint8_t report_id,
		TcPacketBase* current_packet, ReturnValue_t error_code, uint8_t step,
		uint32_t parameter1, uint32_t parameter2) {
	if (this->acknowledgeQueue == 0) {
		this->initialize();
	}
	PusVerificationMessage message(report_id,
			current_packet->getAcknowledgeFlags(),
			current_packet->getPacketId(),
			current_packet->getPacketSequenceControl(), error_code, step,
			parameter1, parameter2);
	ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
	if (status != HasReturnvaluesIF::RETURN_OK) {
		sif::error
				<< "VerificationReporter::sendFailureReport Error writing to queue. Code: "
				<< (uint16_t) status << std::endl;
	}
}

void VerificationReporter::sendFailureReport(uint8_t report_id,
		uint8_t ackFlags, uint16_t tcPacketId, uint16_t tcSequenceControl,
		ReturnValue_t error_code, uint8_t step, uint32_t parameter1,
		uint32_t parameter2) {
	if (this->acknowledgeQueue == 0) {
		this->initialize();
	}
	PusVerificationMessage message(report_id, ackFlags, tcPacketId,
			tcSequenceControl, error_code, step, parameter1, parameter2);
	ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue, &message);
	if (status != HasReturnvaluesIF::RETURN_OK) {
		sif::error
				<< "VerificationReporter::sendFailureReport Error writing to queue. Code: "
				<< (uint16_t) status << std::endl;
	}
}

void VerificationReporter::initialize() {
	AcceptsVerifyMessageIF* temp = objectManager->get<AcceptsVerifyMessageIF>(
			messageReceiver);
	if (temp != NULL) {
		this->acknowledgeQueue = temp->getVerificationQueue();
	} else {
		sif::error
				<< "VerificationReporter::VerificationReporter: Configuration error."
				<< std::endl;
	}
}
