#ifndef SERIALBUFFERADAPTER_H_
#define SERIALBUFFERADAPTER_H_

#include <framework/serialize/SerializeIF.h>
#include <framework/serialize/SerializeAdapter.h>

/**
 * \ingroup serialize
 */
template<typename T>
class SerialBufferAdapter: public SerializeIF {
public:
	SerialBufferAdapter(const uint8_t * buffer, T bufferLength, bool serializeLenght = false);
	SerialBufferAdapter(uint8_t* buffer, T bufferLength,
			bool serializeLenght = false);

	virtual ~SerialBufferAdapter();

	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			size_t maxSize, Endianness streamEndianness) const override;

	virtual size_t getSerializedSize() const override;

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
			Endianness streamEndianness) override;
private:
	bool serializeLength;
	const uint8_t *constBuffer;
	uint8_t *buffer;
	T bufferLength;
};



#endif /* SERIALBUFFERADAPTER_H_ */
