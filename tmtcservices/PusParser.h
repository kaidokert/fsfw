#ifndef FRAMEWORK_TMTCSERVICES_PUSPARSER_H_
#define FRAMEWORK_TMTCSERVICES_PUSPARSER_H_

#include <framework/container/FIFO.h>

#include <utility>
#include <cstdint>

/**
 * @brief	This small helper class scans a given buffer for PUS packets.
 * 			Can be used if PUS packets are serialized in a tightly packed frame.
 * @details
 * The parser uses the payload length field of PUS packets to find
 * the respective PUS packet sizes.
 *
 * The parser parses a buffer by taking a pointer and the maximum size to scan.
 * If PUS packets are found, they are stored in a FIFO which stores pairs
 * consisting of the index in the buffer and the respective packet sizes.
 *
 * If the parser detects split packets (which means that the size of the
 * next packet is larger than the remaining size to scan), it can either
 * store that split packet or throw away the packet.
 */
class PusParser {
public:
	static constexpr uint8_t INTERFACE_ID = CLASS_ID::PUS_PARSER;
	static constexpr ReturnValue_t NO_PACKET_FOUND = MAKE_RETURN_CODE(0x00);

	/**
	 * Parser constructor.
	 * @param maxExpectedPusPackets
	 * Maximum expected number of PUS packets. A good estimate is to divide
	 * the frame size by the minimum size of a PUS packet (12 bytes)
	 * @param storeSplitPackets
	 * Specifies whether split packets are also stored inside the FIFO,
	 * with the size being the remaining frame size.
	 */
	PusParser(uint16_t maxExpectedPusPackets, bool storeSplitPackets);

	/**
	 * Parse a given frame for PUS packets
	 * @param frame
	 * @param frameSize
	 * @return -@c NO_PACKET_FOUND if no packet was found.
	 */
	ReturnValue_t parsePusPackets(const uint8_t* frame, size_t frameSize);
private:
	//! The first entry is the index inside the buffer while the second index
	//! is the size of the PUS packet starting at that index.
	using indexSizePair = std::pair<size_t, size_t>;
	//! A FIFO is used to store information about multiple PUS packets
	//! inside the receive buffer. The maximum number of entries is defined
	//! by the first constructor argument.
	fsfw::FIFO<indexSizePair> indexSizePairFIFO;

	bool storeSplitPackets = false;

	ReturnValue_t readMultiplePackets(size_t frameSize);
};


#endif /* FRAMEWORK_TMTCSERVICES_PUSPARSER_H_ */
