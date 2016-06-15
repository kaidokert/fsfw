/*
 * RMAPStack.h
 *
 *  Created on: 30.05.2013
 *      Author: tod
 */

#ifndef RMAPCHANNEL_H_
#define RMAPCHANNEL_H_

#include <framework/objectmanager/SystemObject.h>
#include <framework/rmap/RMAPChannelIF.h>
extern "C" {
#include <bsp_flp/spw/spw.h>
}
#include <config/datapool/dataPoolInit.h>
#include <framework/devicehandlers/DeviceCommunicationIF.h>
#include <framework/events/Event.h>

class RmapSPWChannel: public SystemObject,
		public RMAPChannelIF,
		public DeviceCommunicationIF {
public:
	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::CDH_1;
	static const Event SPW_ERROR = MAKE_EVENT(0, SEVERITY::LOW);
	static const Event SPW_LINK_DOWN = MAKE_EVENT(1, SEVERITY::LOW);
//	static const Event SPW_CREDIT = MAKE_EVENT(1, SEVERITY::LOW);
//	static const Event SPW_ESCAPE = MAKE_EVENT(2, SEVERITY::LOW);
//	static const Event SPW_DISCONNECT = MAKE_EVENT(3, SEVERITY::LOW);
//	static const Event SPW_PARITY = MAKE_EVENT(4, SEVERITY::LOW);
//	static const Event SPW_WRITE_SYNC = MAKE_EVENT(5, SEVERITY::LOW);
//	static const Event SPW_INVALID_ADDRESS = MAKE_EVENT(6, SEVERITY::LOW);
//	static const Event SPW_EARLY_EOP = MAKE_EVENT(7, SEVERITY::LOW);
//	static const Event SPW_DMA = MAKE_EVENT(8, SEVERITY::LOW);
//	static const Event SPW_LINK_ERROR = MAKE_EVENT(9, SEVERITY::LOW);
	static const Event RMAP_PROTOCOL_ERROR = MAKE_EVENT(10, SEVERITY::LOW);
//	static const Event RMAP_CRC = MAKE_EVENT(10, SEVERITY::LOW);
//	static const Event RMAP_DST_ADDR = MAKE_EVENT(11, SEVERITY::LOW);
//	static const Event RMAP_EEP = MAKE_EVENT(12, SEVERITY::LOW);
//	static const Event RMAP_INSTR = MAKE_EVENT(13, SEVERITY::LOW);
//	static const Event RMAP_INSTR_TYPE = MAKE_EVENT(14, SEVERITY::LOW);
//	static const Event RMAP_LEN = MAKE_EVENT(15, SEVERITY::LOW);
//	static const Event RMAP_LEN_MISSMATCH = MAKE_EVENT(16, SEVERITY::LOW);
//	static const Event RMAP_PROTO = MAKE_EVENT(17, SEVERITY::LOW);
//	static const Event RMAP_SRC_ADDR = MAKE_EVENT(18, SEVERITY::LOW);
//	static const Event RMAP_TRUNC = MAKE_EVENT(19, SEVERITY::LOW);
//	static const Event RMAP_WRONG_REPLY = MAKE_EVENT(20, SEVERITY::LOW);
	static const Event RMAP_MISSED_REPLIES = MAKE_EVENT(21, SEVERITY::LOW);
	static const Event RMAP_NO_RX_DESCRIPTORS = MAKE_EVENT(22, SEVERITY::LOW);
	static const Event RMAP_NO_TX_DESCRIPTORS = MAKE_EVENT(23, SEVERITY::LOW);
	static const Event RMAP_SWITCHED_PORT = MAKE_EVENT(24, SEVERITY::INFO);

	RmapSPWChannel(object_id_t setObjectId, uint16_t buffersize_words,
			uint32_t maxPacketSize, int8_t portNr, uint8_t dest_addr,
			uint8_t src_addr, datapool::opus_variable_id portVariable =
					datapool::NO_PARAMETER);

	virtual ~RmapSPWChannel();

	virtual ReturnValue_t reset();

	virtual ReturnValue_t isActive();

	virtual ReturnValue_t setPort(int8_t port, uint8_t dest_addr,
			uint8_t src_addr);

	virtual ReturnValue_t setPort(int8_t port);

	virtual ReturnValue_t sendCommand(RMAPCookie *cookie, uint8_t instruction,
			uint8_t *data, uint32_t datalen);
	virtual ReturnValue_t getReply(RMAPCookie *cookie, uint8_t **databuffer,
			uint32_t *len);
	virtual ReturnValue_t sendCommandBlocking(RMAPCookie *cookie, uint8_t *data,
			uint32_t datalen, uint8_t **databuffer, uint32_t *len,
			uint32_t timeout_us);

	virtual ReturnValue_t open(Cookie **cookie, uint32_t address,
			uint32_t maxReplyLen);

	virtual ReturnValue_t reOpen(Cookie *cookie, uint32_t address,
			uint32_t maxReplyLen);

	virtual void close(Cookie *cookie);

	virtual ReturnValue_t sendMessage(Cookie *cookie, uint8_t *data,
			uint32_t len);

	virtual ReturnValue_t getSendSuccess(Cookie *cookie);

	virtual ReturnValue_t requestReceiveMessage(Cookie *cookie);

	virtual ReturnValue_t readReceivedMessage(Cookie *cookie, uint8_t **buffer,
			uint32_t *size);

	virtual ReturnValue_t setAddress(Cookie *cookie, uint32_t address);

	virtual uint32_t getAddress(Cookie *cookie);

	virtual ReturnValue_t setParameter(Cookie *cookie, uint32_t parameter);

	virtual uint32_t getParameter(Cookie *cookie);

private:

	SPW_dev *port;
	uint32_t *buffer;
	uint32_t *buffer_pointer; //points to the next free space in the buffer
	uint32_t *end_of_buffer; //points to the word AFTER the buffer
	spw_tx_desc *tx_descriptor_table;
	spw_rx_desc *rx_descriptor_table;
	uint16_t *failure_table;
	uint32_t max_packet_len;
	uint16_t tid;
	uint8_t port_has_crc;
	uint8_t rx_index; //index of the next unused rx descriptor
	uint8_t max_rx;
	uint8_t tx_index; //index of the next unused tx descriptor
	uint8_t max_tx;
	uint8_t src_addr;
	uint8_t dest_addr;

	datapool::opus_variable_id portPoolId;

	static uint8_t null_crc;

	spw_rx_desc *findReply(RMAPCookie *cookie);
	ReturnValue_t checkTxDesc(RMAPCookie *cookie);
	spw_rx_desc *checkRxDesc(RMAPCookie *cookie, uint8_t rxdesc_index);
	void checkRxDescPacket(spw_rx_desc *rxdesc, uint16_t *failureEntry);

	ReturnValue_t checkCrc(uint8_t *packet, uint32_t len);

	void checkRmapHeader(void *_header, uint32_t len, uint16_t *failureEntry);

	void reportFailures(uint16_t failureEntry, uint8_t descriptorNr);
	void reportSpwstr(uint32_t spwstr);
	int8_t getCurrentPortNr();

};

#endif /* RMAPCHANNEL_H_ */
