/**
 * @file	TcTransferFrameLocal.cpp
 * @brief	This file defines the TcTransferFrameLocal class.
 * @date	27.04.2013
 * @author	baetz
 */

#include <framework/datalinklayer/TcTransferFrameLocal.h>
#include <framework/globalfunctions/crc_ccitt.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <string.h>

TcTransferFrameLocal::TcTransferFrameLocal(bool bypass, bool controlCommand, uint16_t scid,
		uint8_t vcId, uint8_t sequenceNumber, uint8_t setSegmentHeader, uint8_t* data, uint16_t dataSize, uint16_t forceCrc) {
	this->frame = (tc_transfer_frame*)&localData;
	frame->header.flagsAndScid = (bypass << 5) + (controlCommand << 4) + ((scid & 0x0300) >> 8);
	frame->header.spacecraftId_l = (scid & 0x00FF);
	frame->header.vcidAndLength_h = (vcId & 0b00111111) << 2;
	frame->header.length_l = sizeof(TcTransferFramePrimaryHeader) -1;
	frame->header.sequenceNumber = sequenceNumber;
	frame->dataField = setSegmentHeader;
	if (data != NULL) {
		if (bypass && controlCommand) {
			memcpy(&(frame->dataField), data, dataSize);
			uint16_t totalSize = sizeof(TcTransferFramePrimaryHeader) + dataSize + FRAME_CRC_SIZE -1;
			frame->header.vcidAndLength_h |= (totalSize & 0x0300) >> 8;
			frame->header.length_l = (totalSize & 0x00FF);
			uint16_t crc = ::Calculate_CRC(getFullFrame(), getFullSize() -2);
			this->getFullFrame()[getFullSize()-2] = (crc & 0xFF00) >> 8;
			this->getFullFrame()[getFullSize()-1] = (crc & 0x00FF);
		} else if (dataSize <= 1016) {
			memcpy(&(frame->dataField) +1, data, dataSize);
			uint16_t dataCrcSize = sizeof(TcTransferFramePrimaryHeader) + 1 + dataSize + FRAME_CRC_SIZE -1;
			frame->header.vcidAndLength_h |= (dataCrcSize & 0x0300) >> 8;
			frame->header.length_l = (dataCrcSize & 0x00FF);
			uint16_t crc = ::Calculate_CRC(getFullFrame(), getFullSize() -2);
			this->getFullFrame()[getFullSize()-2] = (crc & 0xFF00) >> 8;
			this->getFullFrame()[getFullSize()-1] = (crc & 0x00FF);
		} else {
			debug << "TcTransferFrameLocal: dataSize too large: " << dataSize << std::endl;
		}
	} else {
		//No data in frame
	}
	if (forceCrc != 0 ) {
		localData.data[getFullSize()-2] = (forceCrc & 0xFF00) >> 8;
		localData.data[getFullSize()-1] = (forceCrc & 0x00FF);
	}
}
