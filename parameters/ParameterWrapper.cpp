#include "ParameterWrapper.h"

ParameterWrapper::ParameterWrapper() :
		pointsToStream(false), type(Type::UNKNOWN_TYPE) {
}

ParameterWrapper::ParameterWrapper(Type type, uint8_t rows, uint8_t columns,
		void *data) :
		pointsToStream(false), type(type), rows(rows), columns(columns),
		data(data), readonlyData(data) {
}

ParameterWrapper::ParameterWrapper(Type type, uint8_t rows, uint8_t columns,
		const void *data) :
		pointsToStream(false), type(type), rows(rows), columns(columns),
		data(nullptr), readonlyData(data) {
}

ParameterWrapper::~ParameterWrapper() {
}

ReturnValue_t ParameterWrapper::serialize(uint8_t **buffer, size_t *size,
		size_t maxSize, Endianness streamEndianness) const {
	ReturnValue_t result;

	result = SerializeAdapter::serialize(&type, buffer, size, maxSize,
			streamEndianness);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	result = SerializeAdapter::serialize(&columns, buffer, size, maxSize,
			streamEndianness);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = SerializeAdapter::serialize(&rows, buffer, size, maxSize,
			streamEndianness);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	//serialize uses readonlyData, as it is always valid
	if (readonlyData == NULL) {
		return NOT_SET;
	}
	switch (type) {
	case Type::UINT8_T:
		result = serializeData<uint8_t>(buffer, size, maxSize,
				streamEndianness);
		break;
	case Type::INT8_T:
		result = serializeData<int8_t>(buffer, size, maxSize, streamEndianness);
		break;
	case Type::UINT16_T:
		result = serializeData<uint16_t>(buffer, size, maxSize,
				streamEndianness);
		break;
	case Type::INT16_T:
		result = serializeData<int16_t>(buffer, size, maxSize,
				streamEndianness);
		break;
	case Type::UINT32_T:
		result = serializeData<uint32_t>(buffer, size, maxSize,
				streamEndianness);
		break;
	case Type::INT32_T:
		result = serializeData<int32_t>(buffer, size, maxSize,
				streamEndianness);
		break;
	case Type::FLOAT:
		result = serializeData<float>(buffer, size, maxSize, streamEndianness);
		break;
	case Type::DOUBLE:
		result = serializeData<double>(buffer, size, maxSize, streamEndianness);
		break;
	default:
		result = UNKNOW_DATATYPE;
		break;
	}
	return result;
}

size_t ParameterWrapper::getSerializedSize() const {
	uint32_t serializedSize = 0;
	serializedSize += type.getSerializedSize();
	serializedSize += sizeof(rows);
	serializedSize += sizeof(columns);
	serializedSize += rows * columns * type.getSize();

	return serializedSize;
}

template<typename T>
ReturnValue_t ParameterWrapper::serializeData(uint8_t **buffer, size_t *size,
		size_t maxSize, Endianness streamEndianness) const {
	const T *element = (const T*) readonlyData;
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	uint16_t dataSize = columns * rows;
	while (dataSize != 0) {
		result = SerializeAdapter::serialize(element, buffer, size, maxSize,
				streamEndianness);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		element++;
		dataSize--;
	}
	return result;
}

template<typename T>
ReturnValue_t ParameterWrapper::deSerializeData(uint8_t startingRow,
		uint8_t startingColumn, const void *from, uint8_t fromRows,
		uint8_t fromColumns) {

	//treat from as a continuous Stream as we copy all of it
	const uint8_t *fromAsStream = (const uint8_t*) from;
	size_t streamSize = fromRows * fromColumns * sizeof(T);

	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;

	for (uint8_t fromRow = 0; fromRow < fromRows; fromRow++) {

		//get the start element of this row in data
		T *dataWithDataType = ((T*) data)
				+ (((startingRow + fromRow) * columns) + startingColumn);

		for (uint8_t fromColumn = 0; fromColumn < fromColumns; fromColumn++) {
			result = SerializeAdapter::deSerialize(
					dataWithDataType + fromColumn, &fromAsStream, &streamSize,
					SerializeIF::Endianness::BIG);
			if (result != HasReturnvaluesIF::RETURN_OK) {
				return result;
			}
		}
	}

	return result;

}


ReturnValue_t ParameterWrapper::deSerialize(const uint8_t **buffer,
		size_t *size, Endianness streamEndianness) {
	return deSerialize(buffer, size, streamEndianness, 0);
}

ReturnValue_t ParameterWrapper::deSerialize(const uint8_t **buffer,
		size_t *size, Endianness streamEndianness,
		uint16_t startWritingAtIndex) {
	ParameterWrapper streamDescription;

	ReturnValue_t result = streamDescription.set(*buffer, *size, buffer, size);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	return copyFrom(&streamDescription, startWritingAtIndex);
}

ReturnValue_t ParameterWrapper::set(const uint8_t *stream, size_t streamSize,
		const uint8_t **remainingStream, size_t *remainingSize) {
	ReturnValue_t result = SerializeAdapter::deSerialize(&type, &stream,
			&streamSize, SerializeIF::Endianness::BIG);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	result = SerializeAdapter::deSerialize(&columns, &stream, &streamSize,
			SerializeIF::Endianness::BIG);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = SerializeAdapter::deSerialize(&rows, &stream, &streamSize,
			SerializeIF::Endianness::BIG);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	size_t dataSize = type.getSize() * rows * columns;

	if (streamSize < dataSize) {
		return SerializeIF::STREAM_TOO_SHORT;
	}

	data = nullptr;
	readonlyData = stream;
	pointsToStream = true;

	stream += dataSize;
	if (remainingStream !=  nullptr) {
		*remainingStream = stream;
	}
	streamSize -= dataSize;
	if (remainingSize != nullptr) {
		*remainingSize = streamSize;
	}

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ParameterWrapper::copyFrom(const ParameterWrapper *from,
		uint16_t startWritingAtIndex) {
	if (data == NULL) {
		return READONLY;
	}

	if (from->readonlyData == NULL) {
		return SOURCE_NOT_SET;
	}

	if (type != from->type) {
		return DATATYPE_MISSMATCH;
	}

	//check if from fits into this
	uint8_t startingRow = startWritingAtIndex / columns;
	uint8_t startingColumn = startWritingAtIndex % columns;

	if ((from->rows > (rows - startingRow))
			|| (from->columns > (columns - startingColumn))) {
		return TOO_BIG;
	}

	uint8_t typeSize = type.getSize();

	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	//copy data
	if (from->pointsToStream) {
		switch (type) {
		case Type::UINT8_T:
			result = deSerializeData<uint8_t>(startingRow, startingColumn,
					from->readonlyData, from->rows, from->columns);
			break;
		case Type::INT8_T:
			result = deSerializeData<int8_t>(startingRow, startingColumn,
					from->readonlyData, from->rows, from->columns);
			break;
		case Type::UINT16_T:
			result = deSerializeData<uint16_t>(startingRow, startingColumn,
					from->readonlyData, from->rows, from->columns);
			break;
		case Type::INT16_T:
			result = deSerializeData<int16_t>(startingRow, startingColumn,
					from->readonlyData, from->rows, from->columns);
			break;
		case Type::UINT32_T:
			result = deSerializeData<uint32_t>(startingRow, startingColumn,
					from->readonlyData, from->rows, from->columns);
			break;
		case Type::INT32_T:
			result = deSerializeData<int32_t>(startingRow, startingColumn,
					from->readonlyData, from->rows, from->columns);
			break;
		case Type::FLOAT:
			result = deSerializeData<float>(startingRow, startingColumn,
					from->readonlyData, from->rows, from->columns);
			break;
		case Type::DOUBLE:
			result = deSerializeData<double>(startingRow, startingColumn,
					from->readonlyData, from->rows, from->columns);
			break;
		default:
			result = UNKNOW_DATATYPE;
			break;
		}
	}
	else {
		//need a type to do arithmetic
		uint8_t* typedData = static_cast<uint8_t*>(data);
		for (uint8_t fromRow = 0; fromRow < from->rows; fromRow++) {
			size_t offset = (((startingRow + fromRow) * columns) +
					startingColumn) * typeSize;
			std::memcpy(typedData +  offset, from->readonlyData,
					typeSize * from->columns);
		}
	}

	return result;
}
