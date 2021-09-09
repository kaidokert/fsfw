#include "fsfw/globalfunctions/DleEncoder.h"

DleEncoder::DleEncoder(bool escapeStxEtx, bool escapeCr): escapeStxEtx(escapeStxEtx),
        escapeCr(escapeCr) {}

DleEncoder::~DleEncoder() {}

ReturnValue_t DleEncoder::encode(const uint8_t* sourceStream,
		size_t sourceLen, uint8_t* destStream, size_t maxDestLen,
		size_t* encodedLen, bool addStxEtx) {
    size_t minAllowedLen = 0;
    if(escapeStxEtx) {
        minAllowedLen = 1;
    }
    else {
        minAllowedLen = 2;
    }
    if(minAllowedLen > maxDestLen) {
        return STREAM_TOO_SHORT;
    }
	if (addStxEtx) {
	    size_t currentIdx = 0;
	    if(not escapeStxEtx) {
	        destStream[currentIdx++] = DLE_CHAR;
	    }
		destStream[currentIdx] = STX_CHAR;
	}

    if(escapeStxEtx) {
        return encodeStreamEscaped(sourceStream, sourceLen,
                destStream, maxDestLen, encodedLen, addStxEtx);
    }
    else {
        return encodeStreamNonEscaped(sourceStream, sourceLen,
                destStream, maxDestLen, encodedLen, addStxEtx);
    }

}

ReturnValue_t DleEncoder::encodeStreamEscaped(const uint8_t *sourceStream, size_t sourceLen,
        uint8_t *destStream, size_t maxDestLen, size_t *encodedLen,
        bool addStxEtx) {
    size_t encodedIndex = 1;
    size_t sourceIndex = 0;
    uint8_t nextByte = 0;
    while (encodedIndex < maxDestLen and sourceIndex < sourceLen) {
        nextByte = sourceStream[sourceIndex];
        // STX, ETX and CR characters in the stream need to be escaped with DLE
        if ((nextByte == STX_CHAR or nextByte == ETX_CHAR) or
                (this->escapeCr and nextByte == CARRIAGE_RETURN)) {
            if (encodedIndex + 1 >= maxDestLen) {
                return STREAM_TOO_SHORT;
            }
            else {
                destStream[encodedIndex] = DLE_CHAR;
                ++encodedIndex;
                /* Escaped byte will be actual byte + 0x40. This prevents
                 * STX, ETX, and carriage return characters from appearing
                 * in the encoded data stream at all, so when polling an
                 * encoded stream, the transmission can be stopped at ETX.
                 * 0x40 was chosen at random with special requirements:
                 *  - Prevent going from one control char to another
                 *  - Prevent overflow for common characters */
                destStream[encodedIndex] = nextByte + 0x40;
            }
        }
        // DLE characters are simply escaped with DLE.
        else if (nextByte == DLE_CHAR) {
            if (encodedIndex + 1 >= maxDestLen) {
                return STREAM_TOO_SHORT;
            }
            else {
                destStream[encodedIndex] = DLE_CHAR;
                ++encodedIndex;
                destStream[encodedIndex] = DLE_CHAR;
            }
        }
        else {
            destStream[encodedIndex] = nextByte;
        }
        ++encodedIndex;
        ++sourceIndex;
    }

    if (sourceIndex == sourceLen and encodedIndex < maxDestLen) {
        if (addStxEtx) {
            destStream[encodedIndex] = ETX_CHAR;
            ++encodedIndex;
        }
        *encodedLen = encodedIndex;
        return RETURN_OK;
    }
    else {
        return STREAM_TOO_SHORT;
    }
}

ReturnValue_t DleEncoder::encodeStreamNonEscaped(const uint8_t *sourceStream, size_t sourceLen,
        uint8_t *destStream, size_t maxDestLen, size_t *encodedLen,
        bool addStxEtx) {
    size_t encodedIndex = 2;
    size_t sourceIndex = 0;
    uint8_t nextByte = 0;
    while (encodedIndex < maxDestLen and sourceIndex < sourceLen) {
        nextByte = sourceStream[sourceIndex];
        // DLE characters are simply escaped with DLE.
        if (nextByte == DLE_CHAR) {
            if (encodedIndex + 1 >= maxDestLen) {
                return STREAM_TOO_SHORT;
            }
            else {
                destStream[encodedIndex] = DLE_CHAR;
                ++encodedIndex;
                destStream[encodedIndex] = DLE_CHAR;
            }
        }
        else {
            destStream[encodedIndex] = nextByte;
        }
        ++encodedIndex;
        ++sourceIndex;
    }

    if (sourceIndex == sourceLen and encodedIndex < maxDestLen) {
        if (addStxEtx) {
            if(encodedIndex + 2 >= maxDestLen) {
                return STREAM_TOO_SHORT;
            }
            destStream[encodedIndex++] = DLE_CHAR;
            destStream[encodedIndex++] = ETX_CHAR;
        }
        *encodedLen = encodedIndex;
        return RETURN_OK;
    }
    else {
        return STREAM_TOO_SHORT;
    }
}

ReturnValue_t DleEncoder::decode(const uint8_t *sourceStream,
		size_t sourceStreamLen, size_t *readLen, uint8_t *destStream,
		size_t maxDestStreamlen, size_t *decodedLen) {
	size_t encodedIndex = 0;
	if(not escapeStxEtx) {
	    if (*sourceStream != DLE_CHAR) {
	        return DECODING_ERROR;
	    }
	    ++encodedIndex;
	}
	if (sourceStream[encodedIndex] != STX_CHAR) {
	    return DECODING_ERROR;
	}

	if(escapeStxEtx) {
	    return decodeStreamEscaped(sourceStream, sourceStreamLen,
	            readLen, destStream, maxDestStreamlen, decodedLen);
	}
	else {
        return decodeStreamNonEscaped(sourceStream, sourceStreamLen,
                readLen, destStream, maxDestStreamlen, decodedLen);
	}
}

ReturnValue_t DleEncoder::decodeStreamEscaped(const uint8_t *sourceStream, size_t sourceStreamLen,
        size_t *readLen, uint8_t *destStream,
        size_t maxDestStreamlen, size_t *decodedLen) {
    // Skip start marker, was already checked
    size_t encodedIndex = 1;
    size_t decodedIndex = 0;
    uint8_t nextByte;
    while ((encodedIndex < sourceStreamLen)
            and (decodedIndex < maxDestStreamlen)
            and (sourceStream[encodedIndex] != ETX_CHAR)
            and (sourceStream[encodedIndex] != STX_CHAR)) {
        if (sourceStream[encodedIndex] == DLE_CHAR) {
            if(encodedIndex + 1 >= sourceStreamLen) {
                return DECODING_ERROR;
            }
            nextByte = sourceStream[encodedIndex + 1];
            // The next byte is a DLE character that was escaped by another
            // DLE character, so we can write it to the destination stream.
            if (nextByte == DLE_CHAR) {
                destStream[decodedIndex] = nextByte;
            }
            else {
                /* The next byte is a STX, DTX or 0x0D character which
                 * was escaped by a DLE character. The actual byte was
                 * also encoded by adding + 0x40 to prevent having control chars,
                 * in the stream at all, so we convert it back. */
                if ((nextByte == STX_CHAR + 0x40 or nextByte == ETX_CHAR + 0x40) or
                        (this->escapeCr and nextByte == CARRIAGE_RETURN + 0x40)) {
                    destStream[decodedIndex] = nextByte - 0x40;
                }
                else {
                    return DECODING_ERROR;
                }
            }
            ++encodedIndex;
        }
        else {
            destStream[decodedIndex] = sourceStream[encodedIndex];
        }

        ++encodedIndex;
        ++decodedIndex;
    }
    if (sourceStream[encodedIndex] != ETX_CHAR) {
        *readLen = ++encodedIndex;
        return DECODING_ERROR;
    }
    else {
        *readLen = ++encodedIndex;
        *decodedLen = decodedIndex;
        return RETURN_OK;
    }
}

ReturnValue_t DleEncoder::decodeStreamNonEscaped(const uint8_t *sourceStream,
        size_t sourceStreamLen, size_t *readLen, uint8_t *destStream,
        size_t maxDestStreamlen, size_t *decodedLen) {
    // Skip start marker, was already checked
    size_t encodedIndex = 2;
    size_t decodedIndex = 0;
    uint8_t nextByte;
    while ((encodedIndex < sourceStreamLen) && (decodedIndex < maxDestStreamlen)) {
        if (sourceStream[encodedIndex] == DLE_CHAR) {
            if(encodedIndex + 1 >= sourceStreamLen) {
                return DECODING_ERROR;
            }
            nextByte = sourceStream[encodedIndex + 1];
            if(nextByte == STX_CHAR) {
                *readLen = ++encodedIndex;
                return DECODING_ERROR;
            }
            else if(nextByte == DLE_CHAR) {
                // The next byte is a DLE character that was escaped by another
                // DLE character, so we can write it to the destination stream.
                destStream[decodedIndex] = nextByte;
                ++encodedIndex;
            }
            else if(nextByte == ETX_CHAR) {
                // End of stream reached
                *readLen = encodedIndex + 2;
                *decodedLen = decodedIndex;
                return RETURN_OK;
            }
        }
        else {
            destStream[decodedIndex] = sourceStream[encodedIndex];
        }
        ++encodedIndex;
        ++decodedIndex;
    }
    return DECODING_ERROR;
}

void DleEncoder::setEscapeMode(bool escapeStxEtx) {
    this->escapeStxEtx = escapeStxEtx;
}
