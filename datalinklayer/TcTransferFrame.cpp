/**
 * @file	TcTransferFrame.cpp
 * @brief	This file defines the TcTransferFrame class.
 * @date	27.04.2013
 * @author	baetz
 */



#include "TcTransferFrame.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

TcTransferFrame::TcTransferFrame() {
	 frame = NULL;
}

TcTransferFrame::TcTransferFrame(uint8_t* setData) {
	 this->frame = (tc_transfer_frame*)setData;
}

uint8_t TcTransferFrame::getVersionNumber() {
	 return (this->frame->header.flagsAndScid & 0b11000000) >> 6;
}

bool TcTransferFrame::bypassFlagSet() {
	 return (this->frame->header.flagsAndScid & 0b00100000) != 0;
}

bool TcTransferFrame::controlCommandFlagSet() {
	 return (this->frame->header.flagsAndScid & 0b00010000) != 0;
}

bool TcTransferFrame::spareIsZero() {
	 return ( (this->frame->header.flagsAndScid & 0b00001100) == 0 );
}

uint16_t TcTransferFrame::getSpacecraftId() {
	 return ( (this->frame->header.flagsAndScid & 0b00000011) << 8 ) + this->frame->header.spacecraftId_l;
}

uint8_t TcTransferFrame::getVirtualChannelId() {
	 return (this->frame->header.vcidAndLength_h & 0b11111100) >> 2;
}

uint16_t TcTransferFrame::getFrameLength() {
	 return  ( (this->frame->header.vcidAndLength_h & 0b00000011) << 8 ) + this->frame->header.length_l;
}

uint16_t TcTransferFrame::getDataLength() {
	 return  this->getFrameLength() - this->getHeaderSize() -1 - FRAME_CRC_SIZE + 1; // -1 for the segment header.
}

uint8_t TcTransferFrame::getSequenceNumber() {
	 return this->frame->header.sequenceNumber;
}

uint8_t TcTransferFrame::getSequenceFlags() {
	 return (this->frame->dataField & 0b11000000)>>6;
}

uint8_t TcTransferFrame::getMAPId() {
	 return this->frame->dataField & 0b00111111;
}

uint8_t* TcTransferFrame::getDataField() {
	return &(this->frame->dataField) + 1;
}

uint8_t* TcTransferFrame::getFullFrame() {
	return (uint8_t*)this->frame;
}

uint16_t TcTransferFrame::getFullSize() {
	return this->getFrameLength() + 1;
}

uint16_t TcTransferFrame::getHeaderSize() {
	 return sizeof(frame->header);
}

uint16_t TcTransferFrame::getFullDataLength() {
	return this->getFrameLength() - this->getHeaderSize() - FRAME_CRC_SIZE + 1;
}

uint8_t* TcTransferFrame::getFullDataField() {
	return &frame->dataField;
}

void TcTransferFrame::print() {
	sif::debug << "Raw Frame: " << std::hex << std::endl;
	for (uint16_t count = 0; count < this->getFullSize(); count++ ) {
		sif::debug << (uint16_t)this->getFullFrame()[count] << " ";
	}
	sif::debug << std::dec << std::endl;
//	debug << "Frame Header:" << std::endl;
//	debug << "Version Number: " << std::hex << (uint16_t)this->current_frame.getVersionNumber() << std::endl;
//	debug << "Bypass Flag set?| Ctrl Cmd Flag set?: " << (uint16_t)this->current_frame.bypassFlagSet() << " | " << (uint16_t)this->current_frame.controlCommandFlagSet()  << std::endl;
//	debug << "SCID : " << this->current_frame.getSpacecraftId() << std::endl;
//	debug << "VCID : " << (uint16_t)this->current_frame.getVirtualChannelId() << std::endl;
//	debug << "Frame length: " << std::dec << this->current_frame.getFrameLength() << std::endl;
//	debug << "Sequence Number: " << (uint16_t)this->current_frame.getSequenceNumber() << std::endl;
}
