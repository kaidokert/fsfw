#include <framework/globalfunctions/DleEncoder.h>

DleEncoder::DleEncoder() {}

DleEncoder::~DleEncoder() {}

ReturnValue_t DleEncoder::decode(const uint8_t *sourceStream,
		size_t sourceStreamLen, size_t *readLen, uint8_t *destStream,
		size_t maxDestStreamlen, size_t *decodedLen) {
	size_t encodedIndex = 0, decodedIndex = 0;
	uint8_t nextByte;
	if (*sourceStream != STX) {
		return DECODING_ERROR;
	}
	++encodedIndex;
	while ((encodedIndex < sourceStreamLen) && (decodedIndex < maxDestStreamlen)
			&& (sourceStream[encodedIndex] != ETX)
			&& (sourceStream[encodedIndex] != STX)) {
		if (sourceStream[encodedIndex] == DLE) {
			nextByte = sourceStream[encodedIndex + 1];
			// The next byte is a DLE character that was escaped by another
			// DLE character, so we can write it to the destination stream.
			if (nextByte == DLE) {
				destStream[decodedIndex] = nextByte;
			} else {
			    // The next byte is a STX, DTX or 0x0D character which
			    // was escaped by a DLE character
				if ((nextByte == 0x42) || (nextByte == 0x43)
						|| (nextByte == 0x4D)) {
					destStream[decodedIndex] = nextByte - 0x40;
				} else {
				    return DECODING_ERROR;
				}
			}
			++encodedIndex;
		} else {
			destStream[decodedIndex] = sourceStream[encodedIndex];
		}
		++encodedIndex;
		++decodedIndex;
	}
	if (sourceStream[encodedIndex] != ETX) {
		return DECODING_ERROR;
	} else {
		*readLen = ++encodedIndex;
		*decodedLen = decodedIndex;
		return RETURN_OK;
	}
}

ReturnValue_t DleEncoder::encode(const uint8_t* sourceStream,
		size_t sourceLen, uint8_t* destStream, size_t maxDestLen,
		size_t* encodedLen, bool addStxEtx) {
	if (maxDestLen < 2) {
		return STREAM_TOO_SHORT;
	}
	size_t encodedIndex = 0, sourceIndex = 0;
	uint8_t nextByte;
	if (addStxEtx) {
		destStream[0] = STX;
		++encodedIndex;
	}
	while ((encodedIndex < maxDestLen) && (sourceIndex < sourceLen)) {
		nextByte = sourceStream[sourceIndex];
		if ((nextByte == STX) || (nextByte == ETX) || (nextByte == 0x0D)) {
			if (encodedIndex + 1 >= maxDestLen) {
				return STREAM_TOO_SHORT;
			} else {
				destStream[encodedIndex] = DLE;
				++encodedIndex;
				// Escaped byte will be actual byte + 0x40.
				destStream[encodedIndex] = nextByte + 0x40;
			}
		} else if (nextByte == DLE) {
			if (encodedIndex + 1 >= maxDestLen) {
			    return STREAM_TOO_SHORT;
			} else {
				destStream[encodedIndex] = DLE;
				++encodedIndex;
				destStream[encodedIndex] = DLE;
			}
		} else {
			destStream[encodedIndex] = nextByte;
		}
		++encodedIndex;
		++sourceIndex;
	}
	if ((sourceIndex == sourceLen) && (encodedIndex < maxDestLen)) {
		if (addStxEtx) {
			destStream[encodedIndex] = ETX;
			++encodedIndex;
		}
		*encodedLen = encodedIndex;
		return RETURN_OK;
	} else {
		return STREAM_TOO_SHORT;
	}
}
