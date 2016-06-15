#ifndef SERIALBUFFERADAPTER_H_
#define SERIALBUFFERADAPTER_H_

#include <framework/serialize/SerializeIF.h>

class SerialBufferAdapter: public SerializeIF {
public:
	SerialBufferAdapter(const uint8_t * buffer, uint32_t bufferLength);
	SerialBufferAdapter(uint8_t* buffer, uint32_t bufferLength);

	virtual ~SerialBufferAdapter();

	virtual ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const;

	virtual uint32_t getSerializedSize() const;

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian);
private:
	const uint8_t *constBuffer;
	uint8_t *buffer;
	uint32_t bufferLength;
};

#endif /* SERIALBUFFERADAPTER_H_ */
