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
 * SerialElement<SerialBufferAdapter<T(will be uint8_t mostly)>> serialBufferElement
 *
 * \ingroup serialize
 */
template<typename T>
class SerialBufferAdapter: public SerializeIF {
public:
	SerialBufferAdapter(const uint8_t * buffer, T bufferLength, bool serializeLenght = false);
	SerialBufferAdapter(uint8_t* buffer, T bufferLength,
			bool serializeLenght = false);

	virtual ~SerialBufferAdapter();

	virtual ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const;

	virtual uint32_t getSerializedSize() const;

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian);
private:
	bool serializeLength;
	const uint8_t *constBuffer;
	uint8_t *buffer;
	T bufferLength;
};



#endif /* SERIALBUFFERADAPTER_H_ */
