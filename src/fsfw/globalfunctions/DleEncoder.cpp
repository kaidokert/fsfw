#include "fsfw/globalfunctions/DleEncoder.h"

DleEncoder::DleEncoder(bool escapeStxEtx, bool escapeCr)
    : escapeStxEtx(escapeStxEtx), escapeCr(escapeCr) {}

DleEncoder::~DleEncoder() {}

ReturnValue_t DleEncoder::encode(const uint8_t *sourceStream, size_t sourceLen, uint8_t *destStream,
                                 size_t maxDestLen, size_t *encodedLen, bool addStxEtx) {
  if (escapeStxEtx) {
    return encodeStreamEscaped(sourceStream, sourceLen, destStream, maxDestLen, encodedLen,
                               addStxEtx);
  } else {
    return encodeStreamNonEscaped(sourceStream, sourceLen, destStream, maxDestLen, encodedLen,
                                  addStxEtx);
  }
}

ReturnValue_t DleEncoder::encodeStreamEscaped(const uint8_t *sourceStream, size_t sourceLen,
                                              uint8_t *destStream, size_t maxDestLen,
                                              size_t *encodedLen, bool addStxEtx) {
  size_t encodedIndex = 0;
  size_t sourceIndex = 0;
  uint8_t nextByte = 0;
  if (addStxEtx) {
    if (maxDestLen < 1) {
      return STREAM_TOO_SHORT;
    }
    destStream[encodedIndex++] = STX_CHAR;
  }
  while (encodedIndex < maxDestLen and sourceIndex < sourceLen) {
    nextByte = sourceStream[sourceIndex];
    // STX, ETX and CR characters in the stream need to be escaped with DLE
    if ((nextByte == STX_CHAR or nextByte == ETX_CHAR) or
        (this->escapeCr and nextByte == CARRIAGE_RETURN)) {
      if (encodedIndex + 1 >= maxDestLen) {
        return STREAM_TOO_SHORT;
      } else {
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
      } else {
        destStream[encodedIndex] = DLE_CHAR;
        ++encodedIndex;
        destStream[encodedIndex] = DLE_CHAR;
      }
    } else {
      destStream[encodedIndex] = nextByte;
    }
    ++encodedIndex;
    ++sourceIndex;
  }

  if (sourceIndex == sourceLen) {
    if (addStxEtx) {
      if (encodedIndex + 1 >= maxDestLen) {
        return STREAM_TOO_SHORT;
      }
      destStream[encodedIndex] = ETX_CHAR;
      ++encodedIndex;
    }
    *encodedLen = encodedIndex;
    return returnvalue::OK;
  } else {
    return STREAM_TOO_SHORT;
  }
}

ReturnValue_t DleEncoder::encodeStreamNonEscaped(const uint8_t *sourceStream, size_t sourceLen,
                                                 uint8_t *destStream, size_t maxDestLen,
                                                 size_t *encodedLen, bool addStxEtx) {
  size_t encodedIndex = 0;
  size_t sourceIndex = 0;
  uint8_t nextByte = 0;
  if (addStxEtx) {
    if (maxDestLen < 2) {
      return STREAM_TOO_SHORT;
    }
    destStream[encodedIndex++] = DLE_CHAR;
    destStream[encodedIndex++] = STX_CHAR;
  }
  while (encodedIndex < maxDestLen and sourceIndex < sourceLen) {
    nextByte = sourceStream[sourceIndex];
    // DLE characters are simply escaped with DLE.
    if (nextByte == DLE_CHAR) {
      if (encodedIndex + 1 >= maxDestLen) {
        return STREAM_TOO_SHORT;
      } else {
        destStream[encodedIndex] = DLE_CHAR;
        ++encodedIndex;
        destStream[encodedIndex] = DLE_CHAR;
      }
    } else {
      destStream[encodedIndex] = nextByte;
    }
    ++encodedIndex;
    ++sourceIndex;
  }

  if (sourceIndex == sourceLen) {
    if (addStxEtx) {
      if (encodedIndex + 2 >= maxDestLen) {
        return STREAM_TOO_SHORT;
      }
      destStream[encodedIndex++] = DLE_CHAR;
      destStream[encodedIndex++] = ETX_CHAR;
    }
    *encodedLen = encodedIndex;
    return returnvalue::OK;
  } else {
    return STREAM_TOO_SHORT;
  }
}

ReturnValue_t DleEncoder::decode(const uint8_t *sourceStream, size_t sourceStreamLen,
                                 size_t *readLen, uint8_t *destStream, size_t maxDestStreamlen,
                                 size_t *decodedLen) {
  if (escapeStxEtx) {
    return decodeStreamEscaped(sourceStream, sourceStreamLen, readLen, destStream, maxDestStreamlen,
                               decodedLen);
  } else {
    return decodeStreamNonEscaped(sourceStream, sourceStreamLen, readLen, destStream,
                                  maxDestStreamlen, decodedLen);
  }
}

ReturnValue_t DleEncoder::decodeStreamEscaped(const uint8_t *sourceStream, size_t sourceStreamLen,
                                              size_t *readLen, uint8_t *destStream,
                                              size_t maxDestStreamlen, size_t *decodedLen) {
  size_t encodedIndex = 0;
  size_t decodedIndex = 0;
  uint8_t nextByte;

  // init to 0 so that we can just return in the first checks (which do not consume anything from
  // the source stream)
  *readLen = 0;

  if (maxDestStreamlen < 1) {
    return STREAM_TOO_SHORT;
  }
  if (sourceStream[encodedIndex++] != STX_CHAR) {
    return DECODING_ERROR;
  }
  while ((encodedIndex < sourceStreamLen) and (decodedIndex < maxDestStreamlen)) {
    switch (sourceStream[encodedIndex]) {
      case (DLE_CHAR): {
        if (encodedIndex + 1 >= sourceStreamLen) {
          // reached the end of the sourceStream
          *readLen = sourceStreamLen;
          return DECODING_ERROR;
        }
        nextByte = sourceStream[encodedIndex + 1];
        // The next byte is a DLE character that was escaped by another
        // DLE character, so we can write it to the destination stream.
        if (nextByte == DLE_CHAR) {
          destStream[decodedIndex] = nextByte;
        } else {
          /* The next byte is a STX, DTX or 0x0D character which
           * was escaped by a DLE character. The actual byte was
           * also encoded by adding + 0x40 to prevent having control chars,
           * in the stream at all, so we convert it back. */
          if ((nextByte == STX_CHAR + 0x40 or nextByte == ETX_CHAR + 0x40) or
              (this->escapeCr and nextByte == CARRIAGE_RETURN + 0x40)) {
            destStream[decodedIndex] = nextByte - 0x40;
          } else {
            // Set readLen so user can resume parsing after incorrect data
            *readLen = encodedIndex + 2;
            return DECODING_ERROR;
          }
        }
        ++encodedIndex;
        break;
      }
      case (STX_CHAR): {
        *readLen = encodedIndex;
        return DECODING_ERROR;
      }
      case (ETX_CHAR): {
        *readLen = ++encodedIndex;
        *decodedLen = decodedIndex;
        return returnvalue::OK;
      }
      default: {
        destStream[decodedIndex] = sourceStream[encodedIndex];
        break;
      }
    }
    ++encodedIndex;
    ++decodedIndex;
  }

  if (decodedIndex == maxDestStreamlen) {
    // so far we did not find anything wrong here, so let user try again
    *readLen = 0;
    return STREAM_TOO_SHORT;
  } else {
    *readLen = encodedIndex;
    return DECODING_ERROR;
  }
}

ReturnValue_t DleEncoder::decodeStreamNonEscaped(const uint8_t *sourceStream,
                                                 size_t sourceStreamLen, size_t *readLen,
                                                 uint8_t *destStream, size_t maxDestStreamlen,
                                                 size_t *decodedLen) {
  size_t encodedIndex = 0;
  size_t decodedIndex = 0;
  uint8_t nextByte;

  // init to 0 so that we can just return in the first checks (which do not consume anything from
  // the source stream)
  *readLen = 0;

  if (maxDestStreamlen < 2) {
    return STREAM_TOO_SHORT;
  }
  if (sourceStream[encodedIndex++] != DLE_CHAR) {
    return DECODING_ERROR;
  }
  if (sourceStream[encodedIndex++] != STX_CHAR) {
    *readLen = 1;
    return DECODING_ERROR;
  }
  while ((encodedIndex < sourceStreamLen) && (decodedIndex < maxDestStreamlen)) {
    if (sourceStream[encodedIndex] == DLE_CHAR) {
      if (encodedIndex + 1 >= sourceStreamLen) {
        *readLen = encodedIndex;
        return DECODING_ERROR;
      }
      nextByte = sourceStream[encodedIndex + 1];
      if (nextByte == STX_CHAR) {
        // Set readLen so the DLE/STX char combination is preserved. Could be start of
        // another frame
        *readLen = encodedIndex;
        return DECODING_ERROR;
      } else if (nextByte == DLE_CHAR) {
        // The next byte is a DLE character that was escaped by another
        // DLE character, so we can write it to the destination stream.
        destStream[decodedIndex] = nextByte;
        ++encodedIndex;
      } else if (nextByte == ETX_CHAR) {
        // End of stream reached
        *readLen = encodedIndex + 2;
        *decodedLen = decodedIndex;
        return returnvalue::OK;
      } else {
        *readLen = encodedIndex;
        return DECODING_ERROR;
      }
    } else {
      destStream[decodedIndex] = sourceStream[encodedIndex];
    }
    ++encodedIndex;
    ++decodedIndex;
  }

  if (decodedIndex == maxDestStreamlen) {
    // so far we did not find anything wrong here, so let user try again
    *readLen = 0;
    return STREAM_TOO_SHORT;
  } else {
    *readLen = encodedIndex;
    return DECODING_ERROR;
  }
}

void DleEncoder::setEscapeMode(bool escapeStxEtx) { this->escapeStxEtx = escapeStxEtx; }
