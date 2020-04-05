#ifndef SERIALBUFFERADAPTER_H_
#define SERIALBUFFERADAPTER_H_

#include <framework/serialize/SerializeIF.h>
#include <framework/serialize/SerializeAdapter.h>

/**
 * This adapter provides an interface for SerializeIF to serialize or deserialize
 * buffers with no length header but a known size.
 *
 * Additionally, the buffer length can be serialized too and will be put in front of the serialized buffer.
 *
 * Can be used with SerialLinkedListAdapter by declaring a SerializeElement with
 * SerialElement<SerialBufferAdapter<bufferLengthType(will be uint8_t mostly)>> serialBufferElement.
 * Right now, the SerialBufferAdapter must always be initialized with the buffer and size !
 *
 * \ingroup serialize
 */
template<typename count_t>
class SerialBufferAdapter: public SerializeIF {
public:

	/**
	 * Constructor for constant uint8_t buffer. Length field can be serialized optionally.
	 * Type of length can be supplied as template type.
	 * @param buffer
	 * @param bufferLength
	 * @param serializeLength
	 */
	SerialBufferAdapter(const void* buffer, count_t bufferLength, bool serializeLength = false);

	/**
	 * Constructor for non-constant uint8_t buffer. Length field can be serialized optionally.
	 * Type of length can be supplied as template type.
	 * @param buffer
	 * @param bufferLength
	 * @param serializeLength
	 */
	SerialBufferAdapter(void* buffer, count_t bufferLength, bool serializeLength = false);

	virtual ~SerialBufferAdapter();

	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			const size_t max_size, bool bigEndian) const;

	virtual size_t getSerializedSize() const;

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, ssize_t* size,
			bool bigEndian);

	uint8_t * getBuffer();
	const uint8_t * getConstBuffer();
	void setBuffer(void* buffer_, count_t bufferLength_);
private:
	bool m_serialize_length = false;
	const uint8_t *m_const_buffer = nullptr;
	uint8_t *m_buffer = nullptr;
	count_t m_buffer_length = 0;
};



#endif /* SERIALBUFFERADAPTER_H_ */
