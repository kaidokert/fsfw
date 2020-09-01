#include "DleEncoder.h"

DleEncoder::DleEncoder() {
}

DleEncoder::~DleEncoder() {
}

ReturnValue_t DleEncoder::decode(const uint8_t *sourceStream,
		uint32_t sourceStreamLen, uint32_t *readLen, uint8_t *destStream,
		uint32_t maxDestStreamlen, uint32_t *decodedLen) {
	uint32_t encodedIndex = 0, decodedIndex = 0;
	uint8_t nextByte;
	if (*sourceStream != STX) {
		return RETURN_FAILED;
	}
	++encodedIndex;
	while ((encodedIndex < sourceStreamLen) && (decodedIndex < maxDestStreamlen)
			&& (sourceStream[encodedIndex] != ETX)
			&& (sourceStream[encodedIndex] != STX)) {
		if (sourceStream[encodedIndex] == DLE) {
			nextByte = sourceStream[encodedIndex + 1];
			if (nextByte == 0x10) {
				destStream[decodedIndex] = nextByte;
			} else {
				if ((nextByte == 0x42) || (nextByte == 0x43)
						|| (nextByte == 0x4D)) {
					destStream[decodedIndex] = nextByte - 0x40;
				} else {
					return RETURN_FAILED;
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
		return RETURN_FAILED;
	} else {
		*readLen = ++encodedIndex;
		*decodedLen = decodedIndex;
		return RETURN_OK;
	}
}

ReturnValue_t DleEncoder::encode(const uint8_t* sourceStream,
		uint32_t sourceLen, uint8_t* destStream, uint32_t maxDestLen,
		uint32_t* encodedLen, bool addStxEtx) {
	if (maxDestLen < 2) {
		return RETURN_FAILED;
	}
	uint32_t encodedIndex = 0, sourceIndex = 0;
	uint8_t nextByte;
	if (addStxEtx) {
		destStream[0] = STX;
		++encodedIndex;
	}
	while ((encodedIndex < maxDestLen) && (sourceIndex < sourceLen)) {
		nextByte = sourceStream[sourceIndex];
		if ((nextByte == STX) || (nextByte == ETX) || (nextByte == 0x0D)) {
			if (encodedIndex + 1 >= maxDestLen) {
				return RETURN_FAILED;
			} else {
				destStream[encodedIndex] = DLE;
				++encodedIndex;
				destStream[encodedIndex] = nextByte + 0x40;
			}
		} else if (nextByte == DLE) {
			if (encodedIndex + 1 >= maxDestLen) {
				return RETURN_FAILED;
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
		return RETURN_FAILED;
	}
}
