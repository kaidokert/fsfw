/**
 * @file	MapPacketExtraction.cpp
 * @brief	This file defines the MapPacketExtraction class.
 * @date	26.03.2013
 * @author	baetz
 */

#include <framework/datalinklayer/MapPacketExtraction.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/storagemanager/StorageManagerIF.h>
#include <framework/tmtcpacket/SpacePacketBase.h>
#include <framework/tmtcservices/AcceptsTelecommandsIF.h>
#include <framework/tmtcservices/TmTcMessage.h>

MapPacketExtraction::MapPacketExtraction(uint8_t setMapId,
		object_id_t setPacketDestination) :
		lastSegmentationFlag(NO_SEGMENTATION), mapId(setMapId), packetLength(0), bufferPosition(
				packetBuffer), packetDestination(setPacketDestination), packetStore(
				NULL) {
	memset(packetBuffer, 0, sizeof(packetBuffer));
}

ReturnValue_t MapPacketExtraction::extractPackets(TcTransferFrame* frame) {
	uint8_t segmentationFlag = frame->getSequenceFlags();
	ReturnValue_t status = TOO_SHORT_MAP_EXTRACTION;
	switch (segmentationFlag) {
	case NO_SEGMENTATION:
		status = unpackBlockingPackets(frame);
		break;
	case FIRST_PORTION:
		packetLength = frame->getDataLength();
		if (packetLength <= MAX_PACKET_SIZE) {
			memcpy(packetBuffer, frame->getDataField(), packetLength);
			bufferPosition = &packetBuffer[packetLength];
			status = FRAME_OK;
		} else {
			error
					<< "MapPacketExtraction::extractPackets. Packet too large! Size: "
					<< packetLength << std::endl;
			clearBuffers();
			status = CONTENT_TOO_LARGE;
		}
		break;
	case CONTINUING_PORTION:
	case LAST_PORTION:
		if (lastSegmentationFlag == FIRST_PORTION
				|| lastSegmentationFlag == CONTINUING_PORTION) {
			packetLength += frame->getDataLength();
			if (packetLength <= MAX_PACKET_SIZE) {
				memcpy(bufferPosition, frame->getDataField(),
						frame->getDataLength());
				bufferPosition = &packetBuffer[packetLength];
				if (segmentationFlag == LAST_PORTION) {
					status = sendCompletePacket(packetBuffer, packetLength);
					clearBuffers();
				}
				status = FRAME_OK;
			} else {
				error
						<< "MapPacketExtraction::extractPackets. Packet too large! Size: "
						<< packetLength << std::endl;
				clearBuffers();
				status = CONTENT_TOO_LARGE;
			}
		} else {
			error
					<< "MapPacketExtraction::extractPackets. Illegal segment! Last flag: "
					<< (int) lastSegmentationFlag << std::endl;
			clearBuffers();
			status = ILLEGAL_SEGMENTATION_FLAG;
		}
		break;
	default:
		error
				<< "MapPacketExtraction::extractPackets. Illegal segmentationFlag: "
				<< (int) segmentationFlag << std::endl;
		clearBuffers();
		status = DATA_CORRUPTED;
		break;
	}
	lastSegmentationFlag = segmentationFlag;
	return status;
}

ReturnValue_t MapPacketExtraction::unpackBlockingPackets(
		TcTransferFrame* frame) {
	ReturnValue_t status = TOO_SHORT_BLOCKED_PACKET;
	uint32_t totalLength = frame->getDataLength();
	if (totalLength > MAX_PACKET_SIZE)
		return CONTENT_TOO_LARGE;
	uint8_t* position = frame->getDataField();
	while ((totalLength > SpacePacketBase::MINIMUM_SIZE)) {
		SpacePacketBase packet(position);
		uint32_t packetSize = packet.getFullSize();
		if (packetSize <= totalLength) {
			status = sendCompletePacket(packet.getWholeData(),
					packet.getFullSize());
			totalLength -= packet.getFullSize();
			position += packet.getFullSize();
			status = FRAME_OK;
		} else {
			status = DATA_CORRUPTED;
			totalLength = 0;
		}
	}
	if (totalLength > 0) {
		status = RESIDUAL_DATA;
	}
	return status;
}

ReturnValue_t MapPacketExtraction::sendCompletePacket(uint8_t* data,
		uint32_t size) {
	store_address_t store_id;
	ReturnValue_t status = this->packetStore->addData(&store_id, data, size);
	if (status == RETURN_OK) {
		TmTcMessage message(store_id);
		status = this->tcQueue.sendToDefault(&message);
	}
	return status;
}

void MapPacketExtraction::clearBuffers() {
	memset(packetBuffer, 0, sizeof(packetBuffer));
	bufferPosition = packetBuffer;
	packetLength = 0;
	lastSegmentationFlag = NO_SEGMENTATION;
}

ReturnValue_t MapPacketExtraction::initialize() {
	packetStore = objectManager->get<StorageManagerIF>(objects::TC_STORE);
	AcceptsTelecommandsIF* distributor = objectManager->get<
			AcceptsTelecommandsIF>(packetDestination);
	if ((packetStore != NULL) && (distributor != NULL)) {
		tcQueue.setDefaultDestination(distributor->getRequestQueue());
		return RETURN_OK;
	} else {
		return RETURN_FAILED;
	}
}

void MapPacketExtraction::printPacketBuffer(void) {
	debug << "DLL: packet_buffer contains: " << std::endl;
	for (uint32_t i = 0; i < this->packetLength; ++i) {
		debug << "packet_buffer[" << std::dec << i << "]: 0x" << std::hex
				<< (uint16_t) this->packetBuffer[i] << std::endl;
	}
}

uint8_t MapPacketExtraction::getMapId() const {
	return mapId;
}
