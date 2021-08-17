#ifndef FRAMEWORK_GLOBALFUNCTIONS_DLEENCODER_H_
#define FRAMEWORK_GLOBALFUNCTIONS_DLEENCODER_H_

#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include <cstddef>

/**
 * @brief   This DLE Encoder (Data Link Encoder) can be used to encode and
 *          decode arbitrary data with ASCII control characters
 * @details
 * List of control codes:
 * https://en.wikipedia.org/wiki/C0_and_C1_control_codes
 *
 * This encoder can be used to achieve a basic transport layer when using
 * char based transmission systems.
 * The passed source stream is converted into a encoded stream by adding
 * a STX marker at the start of the stream and an ETX marker at the end of
 * the stream. Any STX, ETX, DLE and CR occurrences in the source stream can be
 * escaped by a DLE character. The encoder also replaces escaped control chars
 * by another char, so STX, ETX and CR should not appear anywhere in the actual
 * encoded data stream.
 *
 * When using a strictly char based reception of packets encoded with DLE,
 * STX can be used to notify a reader that actual data will start to arrive
 * while ETX can be used to notify the reader that the data has ended.
 */
class DleEncoder: public HasReturnvaluesIF {
public:
    /**
     * Create an encoder instance with the given configuration.
     * @param escapeStxEtx
     * @param escapeCr
     */
    DleEncoder(bool escapeStxEtx = true, bool escapeCr = false);
    virtual ~DleEncoder();

    static constexpr uint8_t INTERFACE_ID = CLASS_ID::DLE_ENCODER;
    static constexpr ReturnValue_t STREAM_TOO_SHORT = MAKE_RETURN_CODE(0x01);
    static constexpr ReturnValue_t DECODING_ERROR = MAKE_RETURN_CODE(0x02);

    //! Start Of Text character. First character is encoded stream
    static constexpr uint8_t STX_CHAR = 0x02;
    //! End Of Text character. Last character in encoded stream
    static constexpr uint8_t ETX_CHAR = 0x03;
    //! Data Link Escape character. Used to escape STX, ETX and DLE occurrences
    //! in the source stream.
    static constexpr uint8_t DLE_CHAR = 0x10;
    static constexpr uint8_t CARRIAGE_RETURN = 0x0D;

    /**
     * Encodes the give data stream by preceding it with the STX marker
     * and ending it with an ETX marker. DLE characters inside
     * the stream are escaped by DLE characters. STX, ETX and CR characters can be escaped with a
     * DLE character as well. The escaped characters are also encoded by adding
     * 0x40 (which is reverted in the decoding process). This is performed so the source stream
     * does not have STX/ETX/CR occurrences anymore, so the receiving side can simply parse for
     * start and end markers
     * @param sourceStream
     * @param sourceLen
     * @param destStream
     * @param maxDestLen
     * @param encodedLen
     * @param addStxEtx     Adding STX start marker and ETX end marker can be omitted,
     *                      if they are added manually
     * @param escapeStxEtx  STX and ETX occurrences in the given source stream will be escaped and
     *                      encoded by adding 0x40
     * @param escapeCr      CR characters in the given source stream will be escaped and encoded
     *                      by adding 0x40
     * @return
     */
    ReturnValue_t encode(const uint8_t *sourceStream, size_t sourceLen,
            uint8_t *destStream, size_t maxDestLen, size_t *encodedLen,
            bool addStxEtx = true);

    /**
     * Converts an encoded stream back.
     * @param sourceStream
     * @param sourceStreamLen
     * @param readLen
     * @param destStream
     * @param maxDestStreamlen
     * @param decodedLen
     * @param escapeStxEtx  STX and ETX characters were escaped in the encoded stream and need to
     *                      be decoded back as well
     * @param escapeCr      CR characters were escaped in the encoded stream and need to
     *                      be decoded back as well by subtracting 0x40
     * @return
     */
    ReturnValue_t decode(const uint8_t *sourceStream,
            size_t sourceStreamLen, size_t *readLen, uint8_t *destStream,
            size_t maxDestStreamlen, size_t *decodedLen);

private:

    bool escapeStxEtx;
    bool escapeCr;
};

#endif /* FRAMEWORK_GLOBALFUNCTIONS_DLEENCODER_H_ */
