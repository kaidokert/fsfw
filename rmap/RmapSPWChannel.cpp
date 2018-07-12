#include <framework/rmap/RmapSPWChannel.h>
extern "C" {
#include <bsp_flp/hw_timer/hw_timer.h>
#include <bsp_flp/crc/crc.h>
}

#include <framework/datapool/DataSet.h>
#include <framework/datapool/PoolVariable.h>

//////////////////////////////////////////////////////////////////////////////////
//bits for the failure table
#define	RMAP_RX_FAIL_CHECKED		0
#define RMAP_RX_FAIL_EEP			1
#define RMAP_RX_FAIL_TRUNC			2
#define RMAP_RX_FAIL_LEN			3
#define RMAP_RX_FAIL_CRC			4
#define RMAP_RX_FAIL_DST_ADDR		5
#define RMAP_RX_FAIL_PROTO			6
#define RMAP_RX_FAIL_INSTR_TYPE		7	//this is when the instruction byte is really wrong (must be caused by device)
#define RMAP_RX_FAIL_INSTR			8	//this is when a wrong instruction byte was caused by a wrong instruction sent by the host and detected by the device
#define RMAP_RX_FAIL_SRC_ADDR		9
#define RMAP_RX_FAIL_LEN_MISSMATCH	10
#define RMAP_RX_FAIL_WRONG_REPLY	11	//a read command was answered by a write reply or the other way round
//pad a number so it is word aligned (32bit)
#define BYTES2WORDS(number) (((number) +3)/4)

RmapSPWChannel::RmapSPWChannel(object_id_t setObjectId,
		uint16_t buffersize_words, uint32_t maxPacketSize, int8_t portNr,
		uint8_t dest_addr, uint8_t src_addr, PeriodicTaskIF* currentTask,
		datapool::opus_variable_id portVariable) :
		SystemObject(setObjectId), port(NULL), rx_index(0), max_rx(128), tx_index(
				0), max_tx(64), src_addr(src_addr), dest_addr(dest_addr), portPoolId(
				portVariable) {
	buffer = new uint32_t[buffersize_words];
	buffer_pointer = buffer;
	end_of_buffer = &buffer[buffersize_words];
	max_packet_len = maxPacketSize & 0x1FFFFFC;
	tid = 0;
	failure_table = new uint16_t[128];
	setPort(portNr, dest_addr, src_addr, currentTask);
}

RmapSPWChannel::~RmapSPWChannel() {
	delete[] buffer;
	delete[] failure_table;
}

ReturnValue_t RmapSPWChannel::reset() {
	uint8_t link_down = 0;
	uint8_t missed, i;
	uint32_t state;

	//check if channel has a port
	if (port == NULL) {
		return COMMAND_CHANNEL_DEACTIVATED;
	}

	//check state of SPW
	state = spw_get_state(port);
	if (state != SPW_STATE_RUN) {
		triggerEvent(SPW_LINK_DOWN, getCurrentPortNr());
		link_down++;
	}
	//SPW link errors
	if ((state = (port->SPWSTR & 0x1de)) != 0) {
		reportSpwstr(state);
	}
	//check DMA errors
	if ((state = (port->SPWCHN & 0x180)) != 0) {
		triggerEvent(SPW_ERROR, SPW_DMA, state);
	}

	//hard reset spw
	//only safe way to reset the descriptors

	spw_reset(port);

	spw_reset_rxdesc_table(port);
	spw_reset_txdesc_table(port);

	//reset internal buffer
	buffer_pointer = buffer;

#ifdef LEON
	asm("flush");
#endif

	//check how many packets were received
	missed = 0;

	if ((rx_index != 0)
			&& (rx_descriptor_table[rx_index - 1].word0
					& (1 << SPW_DESC_RX_ENABLE))) {
		//		printf("word 0 = %08lx\n",
		//				rx_descriptor_table[rx_index - 1].word0);
		for (missed = 1; missed < rx_index; missed++) {
			if (!(rx_descriptor_table[rx_index - 1 - missed].word0
					& (1 << SPW_DESC_RX_ENABLE)))
				break;
		}
		triggerEvent(RMAP_MISSED_REPLIES, missed);
		//missed is number of missed replies

	}
	if (missed == rx_index) {
		link_down++;
	}

	for (i = rx_index - missed; i > 0; i--) {
		if (!(failure_table[i - 1] & 1)) {
			checkRxDescPacket(&(rx_descriptor_table[i - 1]),
					&(failure_table[i - 1]));
		}
		if (failure_table[i - 1] & 0xFFFE) {
			reportFailures(failure_table[i - 1], i - 1);
		}
	}

	//reset descriptor counter
	rx_index = 0;
	tx_index = 0;

	//clear all descriptors to get rid of unused ones
	spw_rx_desc *rx_desc;
	i = 0;
	for (rx_desc = rx_descriptor_table; rx_desc < &rx_descriptor_table[max_rx];
			rx_desc++) {
		rx_desc->word0 = 0;
		failure_table[i++] = 0;
	}
	spw_tx_desc *tx_desc;
	for (tx_desc = tx_descriptor_table; tx_desc < &tx_descriptor_table[max_tx];
			tx_desc++) {
		tx_desc->word0 = 0;
	}

	//restart spw
	spw_start(port);

	if (link_down > 1) {
		return LINK_DOWN;
	}
	return RETURN_OK;

}

ReturnValue_t RmapSPWChannel::isActive() {
	if (port == NULL) {
		return COMMAND_CHANNEL_DEACTIVATED;
	}
	uint32_t state = spw_get_state(port);
	if (state != SPW_STATE_RUN) {
		return LINK_DOWN;
	}
	return RETURN_OK;

}

ReturnValue_t RmapSPWChannel::setPort(int8_t portNr, uint8_t dest_addr,
		uint8_t src_addr, PeriodicTaskIF* currentTask) {

	SPW_dev *new_port;

	//only accept ports from a list or -1, which disables channel
	if (portNr >= SPW_devices.len && portNr < -1) {
		return COMMAND_PORT_OUT_OF_RANGE;
	}

	//check if crc is enabled

	if (portNr != -1) {
		new_port = SPW_devices.devices[portNr];
		if (!spw_crc_enabled(new_port)) {
			return NO_HW_CRC;
		}
	} else {
		new_port = NULL;
	}

	//anounce change
	triggerEvent(RMAP_SWITCHED_PORT, portNr, getCurrentPortNr());
	if (portPoolId != 0) {
		DataSet mySet;
		PoolVariable<int8_t> channel(portPoolId, &mySet,
				PoolVariableIF::VAR_WRITE);
		mySet.read();
		channel = portNr;
		mySet.commit(PoolVariableIF::VALID);
	}

	//reset the old port to clear pending errors etc
	reset();
	//assign new port
	this->port = new_port;

	if (new_port == NULL) {
		return RETURN_OK;
	}

//stop the new port
	spw_stop(new_port);
//make sure the new port has the max len set
	new_port->SPWRXL = max_packet_len;
//set the addresses
	this->dest_addr = dest_addr;
	this->src_addr = src_addr;
	new_port->SPWNDR = src_addr;
//set descriptor table
	tx_descriptor_table = spw_get_tx_descriptor_table(new_port);
	rx_descriptor_table = spw_get_rx_descriptor_table(new_port);
	spw_reset_rxdesc_table(new_port);
	spw_reset_txdesc_table(new_port);
//reset the channel
	spw_start(new_port);
	currentTask->sleepFor(10);
	reset();
	return RETURN_OK;
}

ReturnValue_t RmapSPWChannel::setPort(int8_t port, PeriodicTaskIF* currentTask) {
	return setPort(port, this->dest_addr, this->src_addr, currentTask);
}

spw_rx_desc* RmapSPWChannel::findReply(RMAPCookie* cookie) {
	spw_rx_desc *rxdesc;
	uint8_t i;

//look downwards
	for (i = cookie->rxdesc_index; i < 200; i--) {
		if ((rxdesc = checkRxDesc(cookie, i)) != NULL) {
			return rxdesc;
		}
	}
//look upwards
	for (i = cookie->rxdesc_index + 1; i < max_rx; i++) {
		if ((rxdesc = checkRxDesc(cookie, i)) != NULL) {
			return rxdesc;
		}
	}

	return NULL;

}

ReturnValue_t RmapSPWChannel::sendCommand(RMAPCookie* cookie,
		uint8_t instruction, uint8_t* data, uint32_t datalen) {

	uint8_t headerlen = RMAP_COMMAND_HEADER_LEN - 1;

//check if channel has a port
	if (port == NULL) {
		return COMMAND_CHANNEL_DEACTIVATED;
	}

//max send limit
	if ((instruction & (1 << RMAP_COMMAND_BIT_WRITE))
			&& (datalen & ~0xFFFFFF)) {
		printf("application wants to send to much data with cookie at %p\n",
				cookie);
		return TOO_MUCH_DATA;
	}
//max receive limit
	if ((!(instruction & (1 << RMAP_COMMAND_BIT_WRITE)))
			&& (datalen + 1 + RMAP_READ_REPLY_HEADER_LEN > max_packet_len)) {
		printf("application wants to read too much data with cookie at %p\n",
				cookie);
		return TOO_MUCH_DATA;
	}

//check if enough space is left in the buffer
//must be checked against max_packet_len, as you can not tell what hw will do,
//except that it will write this number at most
	if ((buffer_pointer + BYTES2WORDS(max_packet_len))
	> end_of_buffer) {
		printf("buffer is full, cookie is %p\n", cookie);
		return COMMAND_BUFFER_FULL;
	}

//we allow tx descriptor wrap at user discretion, but check we have rx descriptors available
//as we dont wrap them
//note: [tr]x_index will be incremented when done
	if (rx_index >= max_rx) {
		triggerEvent(RMAP_NO_RX_DESCRIPTORS, 0);
		return COMMAND_NO_DESCRIPTORS_AVAILABLE;
	}
	if (tx_index >= max_tx) {
		if (max_tx == 64) {
			tx_index = 0;
		} else {
			triggerEvent(RMAP_NO_TX_DESCRIPTORS, 0);
			return COMMAND_NO_DESCRIPTORS_AVAILABLE;
		}
	}

//check if tx descriptor is activated, which means we wrapped the table
//and came to a descriptor not yet sent
#ifdef LEON
	asm("flush");
#endif
	if (tx_descriptor_table[tx_index].word0 & (1 << SPW_DESC_TX_ENABLE)) {
		return COMMAND_NO_DESCRIPTORS_AVAILABLE;
	}

//prepare header
//set addresses
	cookie->header.source_address = src_addr;
	cookie->header.dest_address = dest_addr;
//set instruction
	cookie->header.instruction = instruction;
//set the tid
	cookie->header.tid_l = tid & 0xff;
	cookie->header.tid_h = (tid >> 8) & 0xff;
	tid++;
//set datalen
	cookie->header.datalen_l = datalen & 0xff;
	cookie->header.datalen_m = (datalen >> 8) & 0xff;
	cookie->header.datalen_h = (datalen >> 16) & 0xff;

//configure rx_descriptor
	rx_descriptor_table[rx_index].word1 = (uint32_t) buffer_pointer;
	rx_descriptor_table[rx_index].word0 = (1 << SPW_DESC_RX_ENABLE);

//inc buffer
	if (instruction & (1 << RMAP_COMMAND_BIT_WRITE)) {
		buffer_pointer += BYTES2WORDS(RMAP_WRITE_REPLY_HEADER_LEN);
	} else {
		buffer_pointer += BYTES2WORDS(RMAP_READ_REPLY_HEADER_LEN + datalen + 1); //+1 for crc
	}

//configure tx_descriptor
	tx_descriptor_table[tx_index].word1 = (uint32_t) &cookie->header;
	if (instruction & (1 << RMAP_COMMAND_BIT_WRITE)) {
		tx_descriptor_table[tx_index].word2 = datalen & 0xFFFFFF;
	} else {
		tx_descriptor_table[tx_index].word2 = 0;
	}
	tx_descriptor_table[tx_index].word3 = (uint32_t) data;

	if (instruction & (1 << RMAP_COMMAND_BIT_WRITE)) {
		tx_descriptor_table[tx_index].word0 = headerlen
				| (1 << SPW_DESC_TX_ENABLE) | (1 << SPW_DESC_TX_CRC_DATA)
				| (1 << SPW_DESC_TX_CRC_HEADER);
	} else {
		tx_descriptor_table[tx_index].word0 = headerlen
				| (1 << SPW_DESC_TX_ENABLE) | (1 << SPW_DESC_TX_CRC_HEADER);
	}

//remember descriptors to find them faster when looking for the reply
	cookie->txdesc = &(tx_descriptor_table[tx_index]);
	cookie->rxdesc_index = rx_index;

	tx_index++;
	rx_index++;

//inform the hw about the new descriptors
	spw_new_rxdesc(port);
	spw_new_txdesc(port);

	return RETURN_OK;

}

ReturnValue_t RmapSPWChannel::getReply(RMAPCookie* cookie, uint8_t** databuffer,
		uint32_t* len) {
	spw_rx_desc *rxdesc;
	ReturnValue_t result;
	uint32_t packetlen;
//was the command sent right
	result = checkTxDesc(cookie);
	if (result != RETURN_OK) {
		return result;
	}
//command was sent
//find the rx descriptor
	if ((rxdesc = findReply(cookie)) == NULL) {
		return REPLY_NO_REPLY;
	}
//reply_header = (rmap_write_reply_header *) rxdesc->word1;
	if (databuffer != NULL) {
		packetlen = (rxdesc->word0 & 0x1FFFFFF);
		*databuffer = ((uint8_t *) rxdesc->word1) + RMAP_READ_REPLY_HEADER_LEN;
		*len = packetlen - RMAP_READ_REPLY_HEADER_LEN - 1;
	}
	result = ((RMAPStructs::rmap_write_reply_header *) (rxdesc->word1))->status;
	//result is an RMAP standard return code, not a ReturnValue_t, thus we check against 0
	//and do MAKE_RETURN_CODE(), or set to RETURN_OK respectively
	//if RETURN_OK == 0, this is not neccessary, but we should not make assumptions on the
	//value of ReturnValues
	if (result != 0) {
		result = ((RMAP::INTERFACE_ID << 8) + (result));
	} else {
		result = RETURN_OK;
	}
	return result;

}

ReturnValue_t RmapSPWChannel::checkTxDesc(RMAPCookie* cookie) {
	RMAPStructs::rmap_cmd_header *header_desc;
	uint16_t tid_desc, tid_cookie;
	uint32_t status;
#ifdef LEON
	asm("flush");
#endif
	if (cookie->txdesc == NULL) {
		return REPLY_NOT_SENT;
	}
	header_desc =
			(RMAPStructs::rmap_cmd_header *) ((spw_tx_desc *) cookie->txdesc)->word1;
	status = ((spw_tx_desc *) cookie->txdesc)->word0
			& ((1 << SPW_DESC_TX_ENABLE) | (1 << SPW_DESC_TX_LINK_ERROR));
	tid_cookie = (cookie->header.tid_l) | (cookie->header.tid_h << 8);
	tid_desc = (header_desc->tid_l) | (header_desc->tid_h << 8);
	if (tid_cookie != tid_desc) {
		//tx descriptor was reused, we have no info so let's assume the best
		return RETURN_OK;
	}
//was command sent or an error seen?
	if (status == (1 << SPW_DESC_TX_ENABLE)) {
		return REPLY_NOT_YET_SENT;
	}
	if (status & (1 << SPW_DESC_TX_LINK_ERROR)) {
		triggerEvent(SPW_ERROR, SPW_LINK_ERROR, 0);
		return REPLY_NOT_SENT;
	}
	return RETURN_OK;
}

spw_rx_desc* RmapSPWChannel::checkRxDesc(RMAPCookie* cookie,
		uint8_t rxdesc_index) {
	uint16_t *failureEntry, tid_desc, tid_cookie;
	spw_rx_desc *rxdesc;
	RMAPStructs::rmap_read_reply_header *reply_header;

	rxdesc = &rx_descriptor_table[rxdesc_index];
	failureEntry = &failure_table[rxdesc_index];

	if (!(*failureEntry)) {
		checkRxDescPacket(rxdesc, failureEntry);
	}
	if (*failureEntry != 1 << RMAP_RX_FAIL_CHECKED) {
		return NULL;
	}
//the packet has been checked and is formally correct
//only thing left is to check tid and instruction:

	reply_header = (RMAPStructs::rmap_read_reply_header *) rxdesc->word1;

	tid_cookie = cookie->header.tid_l | (cookie->header.tid_h << 8);
	tid_desc = reply_header->tid_l | (reply_header->tid_h << 8);
	if (tid_desc != tid_cookie) {
		return NULL;
	}

	if ((cookie->header.instruction & 0x3F) != reply_header->instruction) {
		*failureEntry |= (1 << RMAP_RX_FAIL_WRONG_REPLY);
		return NULL;
	}
	return rxdesc;

}

void RmapSPWChannel::checkRxDescPacket(spw_rx_desc* rxdesc,
		uint16_t* failureEntry) {
	uint32_t statusword, len;

#ifdef LEON
	asm("flush");
#endif

	statusword = rxdesc->word0;

	if ((statusword & (1 << SPW_DESC_RX_ENABLE)) || (statusword == 0)) {
		return;
	}

	*failureEntry = 1 << RMAP_RX_FAIL_CHECKED;

	if (statusword & (1 << SPW_DESC_RX_EEP)) {
		*failureEntry |= (1 << RMAP_RX_FAIL_EEP);
		return;
	}

	if (statusword & (1 << SPW_DESC_RX_TRUNCATED)) {
		*failureEntry |= (1 << RMAP_RX_FAIL_TRUNC);
		return;
	}

	len = statusword & 0x1FFFFFF;
	if (!(len >= 13 || len == 8)) {
		*failureEntry |= (1 << RMAP_RX_FAIL_LEN);
		return;
	}
	if (checkCrc((uint8_t*) rxdesc->word1, len) != RETURN_OK) {
		*failureEntry |= (1 << RMAP_RX_FAIL_CRC);
		return;
	}

	checkRmapHeader((void *) rxdesc->word1, len, failureEntry);

}

ReturnValue_t RmapSPWChannel::checkCrc(uint8_t* packet, uint32_t len) {
	if (len == 8) {
		if (crc_calculate(packet, len) == 0) {
			return RETURN_OK;
		} else {
			return RETURN_FAILED;
		}
	} else {
		if ((crc_calculate(packet, RMAP_READ_REPLY_HEADER_LEN) == 0)
				&& (crc_calculate(packet, len) == 0)) {
			return RETURN_OK;
		} else {
			return RETURN_FAILED;
		}
	}
}

void RmapSPWChannel::checkRmapHeader(void* _header, uint32_t len,
		uint16_t* failureEntry) {
	RMAPStructs::rmap_read_reply_header *header =
			(RMAPStructs::rmap_read_reply_header *) _header;
	uint32_t datalen;

	if (header->dest_address != src_addr) {
		*failureEntry |= (1 << RMAP_RX_FAIL_DST_ADDR);
	}

	if (header->protocol != 0x01) {
		*failureEntry |= (1 << RMAP_RX_FAIL_PROTO);
	}

	if (header->instruction & 0xC0) {
		*failureEntry |= (1 << RMAP_RX_FAIL_INSTR_TYPE);
	} else {	//hope that works...
		if ((!(header->instruction & (1 << RMAP_COMMAND_BIT_REPLY)))
				|| ((header->instruction & (1 << RMAP_COMMAND_BIT_VERIFY))
						&& (!(header->instruction
								& (1 << RMAP_COMMAND_BIT_WRITE))))
				|| (header->instruction & 3)) {
			if (header->status == REPLY_UNUSED_PACKET_TYPE_OR_COMMAND_CODE) {
				*failureEntry |= (1 << RMAP_RX_FAIL_INSTR);
			} else {
				*failureEntry |= (1 << RMAP_RX_FAIL_INSTR_TYPE);
			}
		}
	}

	if (header->instruction & (1 << RMAP_COMMAND_BIT_WRITE)) {
		if (len != 8) {
			*failureEntry |= (1 << RMAP_RX_FAIL_INSTR_TYPE);
		}
	} else {
		if (len == 8) {
			*failureEntry |= (1 << RMAP_RX_FAIL_INSTR_TYPE);
		}
	}

	if (header->source_address != dest_addr) {
		*failureEntry |= (1 << RMAP_RX_FAIL_SRC_ADDR);
	}

	if (len != 8) {
		datalen = header->datalen_l | (header->datalen_m << 8)
				| (header->datalen_h << 16);
		if (datalen + RMAP_READ_REPLY_HEADER_LEN + 1 != len) {
			*failureEntry |= (1 << RMAP_RX_FAIL_LEN_MISSMATCH);
		}
	}
}

ReturnValue_t RmapSPWChannel::sendCommandBlocking(RMAPCookie *cookie,
		uint8_t *data, uint32_t datalen, uint8_t **databuffer, uint32_t *len,
		uint32_t timeout_us) {
	uint32_t time;
	ReturnValue_t result;
	if (databuffer != NULL) {
		result = sendReadCommand(cookie, datalen);
	} else {
		result = sendWriteCommand(cookie, data, datalen);
	}
	if (result != RETURN_OK) {
		return result;
	}
	time = hw_timer_get_us();
	while (time - hw_timer_get_us() < timeout_us) {
		result = getReply(cookie, databuffer, len);
		if (!((result == REPLY_NOT_YET_SENT) || (result == REPLY_NO_REPLY))) {
			return result;
		}
	}
	if (result == REPLY_NOT_YET_SENT) {
		return result;
	}
	return REPLY_TIMEOUT;
}

//SHOULDDO find a better way to inject the Extended address
#include <config/hardware/IoBoardAddresses.h>
ReturnValue_t RmapSPWChannel::open(Cookie **cookie, uint32_t address,
		uint32_t maxReplyLen) {
	*cookie = new RMAPCookie(address, IoBoardExtendedAddresses::DEVICE_BUFFER,
			this, 0, maxReplyLen);
	return RETURN_OK;
}

ReturnValue_t RmapSPWChannel::reOpen(Cookie* cookie, uint32_t address,
		uint32_t maxReplyLen) {
	ReturnValue_t result = isActive();
	if (result != RETURN_OK) {
		return result;
	}
	RMAPCookie *rCookie = dynamic_cast<RMAPCookie *>(cookie);
	if (rCookie == NULL) {
		return INVALID_COOKIE_TYPE;
	}
	rCookie->setAddress(address);
	rCookie->setChannel(this);
	rCookie->setCommandMask(0);
	rCookie->setExtendedAddress(IoBoardExtendedAddresses::DEVICE_BUFFER);
	rCookie->setMaxReplyLen(maxReplyLen);
	return RETURN_OK;
}

void RmapSPWChannel::close(Cookie* cookie) {
	delete cookie;
}

ReturnValue_t RmapSPWChannel::sendMessage(Cookie* cookie, uint8_t* data,
		uint32_t len) {
	return sendWriteCommand((RMAPCookie *) cookie, data, len);
}

ReturnValue_t RmapSPWChannel::getSendSuccess(Cookie* cookie) {
	return getWriteReply((RMAPCookie *) cookie);
}

ReturnValue_t RmapSPWChannel::requestReceiveMessage(Cookie* cookie) {
	return sendReadCommand((RMAPCookie *) cookie,
			((RMAPCookie *) cookie)->getMaxReplyLen());
}

ReturnValue_t RmapSPWChannel::readReceivedMessage(Cookie* cookie,
		uint8_t** buffer, uint32_t* size) {
	return getReadReply((RMAPCookie *) cookie, buffer, size);
}

ReturnValue_t RmapSPWChannel::setAddress(Cookie* cookie, uint32_t address) {
	((RMAPCookie *) cookie)->setAddress(address);
	return HasReturnvaluesIF::RETURN_OK;
}

uint32_t RmapSPWChannel::getAddress(Cookie* cookie) {
	return ((RMAPCookie *) cookie)->getAddress();
}

ReturnValue_t RmapSPWChannel::setParameter(Cookie* cookie, uint32_t parameter) {
	return RETURN_FAILED;
}

uint32_t RmapSPWChannel::getParameter(Cookie* cookie) {
	return 0;
}

void RmapSPWChannel::reportFailures(uint16_t failureEntry,
		uint8_t descriptorNr) {
	if (failureEntry & (1 << RMAP_RX_FAIL_CRC)) {
		triggerEvent(RMAP_PROTOCOL_ERROR, REPLY_INVALID_DATA_CRC, descriptorNr);
	}
	if (failureEntry & (1 << RMAP_RX_FAIL_DST_ADDR)) {
		triggerEvent(RMAP_PROTOCOL_ERROR, REPLY_INVALID_TARGET_LOGICAL_ADDRESS,
				descriptorNr);
	}
	if (failureEntry & (1 << RMAP_RX_FAIL_EEP)) {
		triggerEvent(RMAP_PROTOCOL_ERROR, REPLY_EEP, descriptorNr);
	}
	if (failureEntry & (1 << RMAP_RX_FAIL_INSTR)) {
		triggerEvent(RMAP_PROTOCOL_ERROR,
				REPLY_COMMAND_NOT_IMPLEMENTED_OR_NOT_AUTHORISED, descriptorNr);
	}
	if (failureEntry & (1 << RMAP_RX_FAIL_INSTR_TYPE)) {
		triggerEvent(RMAP_PROTOCOL_ERROR, REPLY_RESERVED, descriptorNr);
	}
	if (failureEntry & (1 << RMAP_RX_FAIL_LEN)) {
		triggerEvent(RMAP_PROTOCOL_ERROR, REPLY_TOO_MUCH_DATA, descriptorNr);
	}
	if (failureEntry & (1 << RMAP_RX_FAIL_LEN_MISSMATCH)) {
		triggerEvent(RMAP_PROTOCOL_ERROR, REPLY_INVALID_DATA, descriptorNr);
	}
	if (failureEntry & (1 << RMAP_RX_FAIL_PROTO)) {
		triggerEvent(RMAP_PROTOCOL_ERROR, REPLY_GENERAL_ERROR_CODE,
				descriptorNr);
	}
	if (failureEntry & (1 << RMAP_RX_FAIL_SRC_ADDR)) {
		triggerEvent(RMAP_PROTOCOL_ERROR, REPLY_GENERAL_ERROR_CODE,
				descriptorNr);
	}
	if (failureEntry & (1 << RMAP_RX_FAIL_TRUNC)) {
		triggerEvent(RMAP_PROTOCOL_ERROR, REPLY_TRANSMISSION_ERROR,
				descriptorNr);
	}
	if (failureEntry & (1 << RMAP_RX_FAIL_WRONG_REPLY)) {
		triggerEvent(RMAP_PROTOCOL_ERROR, REPLY_INVALID_KEY, descriptorNr);
	}

}

void RmapSPWChannel::reportSpwstr(uint32_t spwstr) {
	if (spwstr & (1 << SPW_SPWSTR_CE)) {
		triggerEvent(SPW_ERROR, SPW_CREDIT);
	}
	if (spwstr & (1 << SPW_SPWSTR_ER)) {
		triggerEvent(SPW_ERROR, SPW_ESCAPE);
	}
	if (spwstr & (1 << SPW_SPWSTR_DE)) {
		triggerEvent(SPW_ERROR, SPW_DISCONNECT);
	}
	if (spwstr & (1 << SPW_SPWSTR_PE)) {
		triggerEvent(SPW_ERROR, SPW_PARITY);
	}
	if (spwstr & (1 << SPW_SPWSTR_WE)) {
		triggerEvent(SPW_ERROR, SPW_WRITE_SYNC);
	}
	if (spwstr & (1 << SPW_SPWSTR_IA)) {
		triggerEvent(SPW_ERROR, SPW_INVALID_ADDRESS);
	}
	if (spwstr & (1 << SPW_SPWSTR_EE)) {
		triggerEvent(SPW_ERROR, SPW_EARLY_EOP);
	}
}

uint8_t RmapSPWChannel::null_crc = 0;

int8_t RmapSPWChannel::getCurrentPortNr() {
	uint8_t i;
	int8_t oldport = -1;
	for (i = 0; i < SPW_devices.len; ++i) {
		if (SPW_devices.devices[i] == this->port) {
			oldport = i;
		}
	}
	return oldport;
}
