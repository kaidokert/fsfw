#ifndef DATAPOOLPARAMETERWRAPPER_H_
#define DATAPOOLPARAMETERWRAPPER_H_

#include "../globalfunctions/Type.h"
#include "../parameters/ParameterWrapper.h"

class DataPoolParameterWrapper: public SerializeIF {
public:
	DataPoolParameterWrapper();
	virtual ~DataPoolParameterWrapper();

	ReturnValue_t set(uint8_t domainId, uint16_t parameterId);

	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			size_t maxSize, Endianness streamEndianness) const override;

	virtual size_t getSerializedSize() const override;

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
			Endianness streamEndianness) override;

	ReturnValue_t copyFrom(const ParameterWrapper *from,
			uint16_t startWritingAtIndex);

private:
	Type type;
	uint8_t rows;
	uint8_t columns;

	uint32_t poolId;

	template<typename T>
	ReturnValue_t deSerializeData(uint8_t startingRow, uint8_t startingColumn,
			const void *from, uint8_t fromRows);

};

#endif /* DATAPOOLPARAMETERWRAPPER_H_ */
