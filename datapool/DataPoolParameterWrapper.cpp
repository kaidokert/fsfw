#include "DataPoolParameterWrapper.h"

//for returncodes
#include "../parameters/HasParametersIF.h"

#include "DataSet.h"
#include "PoolRawAccess.h"

DataPoolParameterWrapper::DataPoolParameterWrapper() :
		type(Type::UNKNOWN_TYPE), rows(0), columns(0), poolId(
				PoolVariableIF::NO_PARAMETER) {

}

DataPoolParameterWrapper::~DataPoolParameterWrapper() {

}

ReturnValue_t DataPoolParameterWrapper::set(uint8_t domainId,
		uint16_t parameterId) {
	poolId = (domainId << 16) + parameterId;

	DataSet mySet;
	PoolRawAccess raw(poolId, 0, &mySet, PoolVariableIF::VAR_READ);
	ReturnValue_t status = mySet.read();
	if (status != HasReturnvaluesIF::RETURN_OK) {
		//should only fail for invalid pool id
		return HasParametersIF::INVALID_MATRIX_ID;
	}

	type = raw.getType();
	rows = raw.getArraySize();
	columns = 1;

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t DataPoolParameterWrapper::serialize(uint8_t** buffer,
		size_t* size, size_t maxSize, Endianness streamEndianness) const {
	ReturnValue_t result;

	result = SerializeAdapter::serialize(&type, buffer, size, maxSize,
			streamEndianness);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	result = SerializeAdapter::serialize(&columns, buffer, size,
			maxSize, streamEndianness);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = SerializeAdapter::serialize(&rows, buffer, size, maxSize,
			streamEndianness);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	for (uint8_t index = 0; index < rows; index++){
		DataSet mySet;
		PoolRawAccess raw(poolId, index, &mySet,PoolVariableIF::VAR_READ);
		mySet.read();
		result = raw.serialize(buffer,size,maxSize,streamEndianness);
		if (result != HasReturnvaluesIF::RETURN_OK){
			return result;
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}

//same as ParameterWrapper
size_t DataPoolParameterWrapper::getSerializedSize() const {
	size_t serializedSize = 0;
	serializedSize += type.getSerializedSize();
	serializedSize += sizeof(rows);
	serializedSize += sizeof(columns);
	serializedSize += rows * columns * type.getSize();

	return serializedSize;
}

ReturnValue_t DataPoolParameterWrapper::deSerialize(const uint8_t** buffer,
		size_t* size, Endianness streamEndianness) {
	return HasReturnvaluesIF::RETURN_FAILED;
}

template<typename T>
ReturnValue_t DataPoolParameterWrapper::deSerializeData(uint8_t startingRow,
		uint8_t startingColumn, const void* from, uint8_t fromRows) {
	//treat from as a continuous Stream as we copy all of it
	const uint8_t *fromAsStream = (const uint8_t *) from;

	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;

	for (uint8_t fromRow = 0; fromRow < fromRows; fromRow++) {

		DataSet mySet;
		PoolRawAccess raw(poolId, startingRow + fromRow, &mySet,
				PoolVariableIF::VAR_READ_WRITE);
		mySet.read();

		result = raw.setEntryFromBigEndian(fromAsStream, sizeof(T));

		fromAsStream += sizeof(T);

		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}

		mySet.commit();
	}

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t DataPoolParameterWrapper::copyFrom(const ParameterWrapper* from,
		uint16_t startWritingAtIndex) {
	if (poolId == PoolVariableIF::NO_PARAMETER) {
		return ParameterWrapper::NOT_SET;
	}

	if (type != from->type) {
		return ParameterWrapper::DATATYPE_MISSMATCH;
	}

	//check if from fits into this
	uint8_t startingRow = startWritingAtIndex / columns;
	uint8_t startingColumn = startWritingAtIndex % columns;

	if ((from->rows > (rows - startingRow))
			|| (from->columns > (columns - startingColumn))) {
		return ParameterWrapper::TOO_BIG;
	}

	ReturnValue_t result;
	//copy data
	if (from->pointsToStream) {
		switch (type) {
		case Type::UINT8_T:
			result = deSerializeData<uint8_t>(startingRow, startingColumn,
					from->readonlyData, from->rows);
			break;
		case Type::INT8_T:
			result = deSerializeData<int8_t>(startingRow, startingColumn,
					from->readonlyData, from->rows);
			break;
		case Type::UINT16_T:
			result = deSerializeData<uint16_t>(startingRow, startingColumn,
					from->readonlyData, from->rows);
			break;
		case Type::INT16_T:
			result = deSerializeData<int16_t>(startingRow, startingColumn,
					from->readonlyData, from->rows);
			break;
		case Type::UINT32_T:
			result = deSerializeData<uint32_t>(startingRow, startingColumn,
					from->readonlyData, from->rows);
			break;
		case Type::INT32_T:
			result = deSerializeData<int32_t>(startingRow, startingColumn,
					from->readonlyData, from->rows);
			break;
		case Type::FLOAT:
			result = deSerializeData<float>(startingRow, startingColumn,
					from->readonlyData, from->rows);
			break;
		case Type::DOUBLE:
			result = deSerializeData<double>(startingRow, startingColumn,
					from->readonlyData, from->rows);
			break;
		default:
			result = ParameterWrapper::UNKNOW_DATATYPE;
			break;
		}
	} else {
		//not supported
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	return result;
}
