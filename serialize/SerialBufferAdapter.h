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
template<typename T>
class SerialBufferAdapter: public SerializeIF {
public:
	/**
	 * Constructor for constant uint8_t buffer. Length field can be serialized optionally.
	 * Type of length can be supplied as template type.
	 * @param buffer
	 * @param bufferLength
	 * @param serializeLength
	 */
	SerialBufferAdapter(const uint8_t * buffer, T bufferLength, bool serializeLength = false);

	/**
	 * Constructor for non-constant uint8_t buffer. Length field can be serialized optionally.
	 * Type of length can be supplied as template type.
	 * @param buffer
	 * @param bufferLength
	 * @param serializeLength
	 */
	SerialBufferAdapter(uint8_t* buffer, T bufferLength, bool serializeLength = false);

	/**
	 * Constructoor for non-constant uint32_t buffer. Length field can be serialized optionally.
	 * Type of length can be supplied as template type.
	 * @param buffer
	 * @param bufferLength
	 * @param serializeLength
	 */
	SerialBufferAdapter(uint32_t* buffer,T bufferLength, bool serializeLength = false);

	virtual ~SerialBufferAdapter();

	virtual ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const;

	virtual uint32_t getSerializedSize() const;

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian);

	uint8_t * getBuffer();
	const uint8_t * getConstBuffer();
	void setBuffer(uint8_t * buffer_, T bufferLength_);
	void setBuffer(uint32_t * buffer_, T bufferLength_);
private:

	enum bufferType {
		NORMAL,
		CONST
	};
	bufferType currentBufferType;

	bool serializeLength;
	const uint8_t *constBuffer;
	uint8_t *buffer;
	T bufferLength;
};



#endif /* SERIALBUFFERADAPTER_H_ */
