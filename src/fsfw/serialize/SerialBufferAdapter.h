#ifndef SERIALBUFFERADAPTER_H_
#define SERIALBUFFERADAPTER_H_

#include "fsfw/serialize/SerializeAdapter.h"
#include "fsfw/serialize/SerializeIF.h"

/**
 * This adapter provides an interface for SerializeIF to serialize or deserialize
 * buffers with no length header but a known size.
 *
 * Additionally, the buffer length can be serialized too and will be put in
 * front of the serialized buffer.
 *
 * Can be used with SerialLinkedListAdapter by declaring a SerializeElement with
 * SerialElement<SerialBufferAdapter<bufferLengthType(will be uint8_t mostly)>>.
 * Right now, the SerialBufferAdapter must always
 * be initialized with the buffer and size !
 *
 * \ingroup serialize
 */
template <typename count_t>
class SerialBufferAdapter : public SerializeIF {
 public:
  SerialBufferAdapter() = default;
  /**
   * Constructor for constant uint8_t buffer. Length field can be serialized optionally.
   * Type of length can be supplied as template type.
   * @param buffer
   * @param bufferLength
   * @param serializeLength
   */
  SerialBufferAdapter(const uint8_t* buffer, count_t bufferLength, bool serializeLength = false);

  /**
   * Constructor for non-constant uint8_t buffer.
   * Length field can be serialized optionally.
   * Type of length can be supplied as template type.
   * @param buffer
   * @param bufferLength
   * @param serializeLength Length field will be serialized with size count_t
   */
  SerialBufferAdapter(uint8_t* buffer, count_t bufferLength, bool serializeLength = false);

  ~SerialBufferAdapter() override;

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

  [[nodiscard]] size_t getSerializedSize() const override;

  /**
   * @brief This function deserializes a buffer into the member buffer.
   * @details
   * If a length field is present, it is ignored, as the size should have
   * been set in the constructor. If the size is not known beforehand,
   * consider using SerialFixedArrayListAdapter instead.
   * @param buffer_ [out] Resulting buffer
   * @param size remaining size to deserialize, should be larger than buffer
   *        + size field size
   * @param bigEndian
   * @return
   */
  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override;

  uint8_t* getBuffer();
  [[nodiscard]] const uint8_t* getConstBuffer() const;
  void setConstBuffer(const uint8_t* buf, count_t bufLen);

 private:
  bool serializeLength = false;
  const uint8_t* constBuffer = nullptr;
  uint8_t* buffer = nullptr;
  count_t bufferLength = 0;
};

#endif /* SERIALBUFFERADAPTER_H_ */
