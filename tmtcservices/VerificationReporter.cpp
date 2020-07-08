#include <framework/tmtcservices/VerificationReporter.h>

#include <framework/ipc/MessageQueueIF.h>
#include <framework/tmtcservices/AcceptsVerifyMessageIF.h>
#include <framework/tmtcservices/PusVerificationReport.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/objectmanager/frameworkObjects.h>

object_id_t VerificationReporter::messageReceiver = objects::PUS_SERVICE_1;

VerificationReporter::VerificationReporter() :
		acknowledgeQueue(MessageQueueIF::NO_QUEUE) {
}

VerificationReporter::~VerificationReporter() {}

void VerificationReporter::sendSuccessReport(uint8_t set_report_id,
		TcPacketBase* current_packet, uint8_t set_step) {
	if (this->acknowledgeQueue == 0) {
		this->initialize();
	}
	PusVerificationMessage message(set_report_id,
			current_packet->getAcknowledgeFlags(),
			current_packet->getPacketId(),
			current_packet->getPacketSequenceControl(), 0, set_step);
	ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue,
			&message);
	if (status != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "VerificationReporter::sendSuccessReport: Error writing "
				<< "to queue. Code: " << std::hex << status << std::dec
				<< std::endl;
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
	ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue,
			&message);
	if (status != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "VerificationReporter::sendSuccessReport: Error writing "
				<< "to queue. Code: " << std::hex << status << std::dec
				<< std::endl;
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
	ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue,
	        &message);
	if (status != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "VerificationReporter::sendFailureReport Error writing "
				<< "to queue. Code: " << std::hex << status << std::dec
				<< std::endl;
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
	ReturnValue_t status = MessageQueueSenderIF::sendMessage(acknowledgeQueue,
	        &message);
	if (status != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "VerificationReporter::sendFailureReport Error writing "
				<< "to queue. Code: " << std::hex << status << std::dec
				<< std::endl;
	}
}

void VerificationReporter::initialize() {
	if(messageReceiver == objects::NO_OBJECT) {
		sif::warning << "VerificationReporter::initialize: Verification message"
				" receiver object ID not set yet in Factory!" << std::endl;
		return;
	}
	AcceptsVerifyMessageIF* temp = objectManager->get<AcceptsVerifyMessageIF>(
			messageReceiver);
	if (temp == nullptr) {
		sif::error << "VerificationReporter::initialize: Message "
				<< "receiver invalid. Make sure it is set up properly and "
				<<"implementsAcceptsVerifyMessageIF" << std::endl;

	}
	this->acknowledgeQueue = temp->getVerificationQueue();
}
