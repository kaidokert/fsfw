#include <framework/rmap/RmapDeviceCommunicationIF.h>
#include <framework/rmap/RMAP.h>

//TODO Cast here are all potential bugs
RmapDeviceCommunicationIF::~RmapDeviceCommunicationIF() {
}

ReturnValue_t RmapDeviceCommunicationIF::sendMessage(Cookie* cookie,
		uint8_t* data, uint32_t len) {
	return RMAP::sendWriteCommand((RMAPCookie *) cookie, data, len);
}

ReturnValue_t RmapDeviceCommunicationIF::getSendSuccess(Cookie* cookie) {
	return RMAP::getWriteReply((RMAPCookie *) cookie);
}

ReturnValue_t RmapDeviceCommunicationIF::requestReceiveMessage(
		Cookie* cookie) {
	return RMAP::sendReadCommand((RMAPCookie *) cookie,
				((RMAPCookie *) cookie)->getMaxReplyLen());
}

ReturnValue_t RmapDeviceCommunicationIF::readReceivedMessage(Cookie* cookie,
		uint8_t** buffer, uint32_t* size) {
	return RMAP::getReadReply((RMAPCookie *) cookie, buffer, size);
}

ReturnValue_t RmapDeviceCommunicationIF::setAddress(Cookie* cookie,
		uint32_t address) {

	((RMAPCookie *) cookie)->setAddress(address);
	return HasReturnvaluesIF::RETURN_OK;
}

uint32_t RmapDeviceCommunicationIF::getAddress(Cookie* cookie) {
	return ((RMAPCookie *) cookie)->getAddress();
}

ReturnValue_t RmapDeviceCommunicationIF::setParameter(Cookie* cookie,
		uint32_t parameter) {
	//TODO Empty?
	return HasReturnvaluesIF::RETURN_FAILED;
}

uint32_t RmapDeviceCommunicationIF::getParameter(Cookie* cookie) {
	return 0;
}
