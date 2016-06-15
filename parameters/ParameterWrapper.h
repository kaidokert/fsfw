#ifndef PARAMETERWRAPPER_H_
#define PARAMETERWRAPPER_H_

#include <framework/globalfunctions/Type.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/serialize/SerializeAdapter.h>
#include <framework/serialize/SerializeIF.h>
#include <stddef.h>
#include <framework/globalfunctions/Type.h>

class ParameterWrapper: public SerializeIF {
public:
	static const uint8_t INTERFACE_ID = PARAMETER_WRAPPER;
	static const ReturnValue_t UNKNOW_DATATYPE = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t DATATYPE_MISSMATCH = MAKE_RETURN_CODE(0x02);
	static const ReturnValue_t READONLY = MAKE_RETURN_CODE(0x03);
	static const ReturnValue_t TOO_BIG = MAKE_RETURN_CODE(0x04);
	static const ReturnValue_t SOURCE_NOT_SET = MAKE_RETURN_CODE(0x05);
	static const ReturnValue_t OUT_OF_BOUNDS = MAKE_RETURN_CODE(0x06);
	static const ReturnValue_t NOT_SET = MAKE_RETURN_CODE(0x07);

	ParameterWrapper();
	ParameterWrapper(Type type, uint8_t rows, uint8_t columns, void *data);
	ParameterWrapper(Type type, uint8_t rows, uint8_t columns,
			const void *data);
	virtual ~ParameterWrapper();

	virtual ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const;

	virtual uint32_t getSerializedSize() const;

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian);

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian, uint16_t startWritingAtIndex = 0);

	template<typename T>
	ReturnValue_t getElement(T *value, uint8_t row = 0, uint8_t column = 0) const {
		if (readonlyData == NULL){
			return NOT_SET;
		}

		if (PodTypeConversion<T>::type != type) {
			return DATATYPE_MISSMATCH;
		}

		if ((row > rows) || (column > columns)) {
			return OUT_OF_BOUNDS;
		}

		if (pointsToStream) {
			const uint8_t *streamWithtype = (const uint8_t *) readonlyData;
			streamWithtype += (row * columns + column) * type.getSize();
			int32_t size = type.getSize();
			return SerializeAdapter<T>::deSerialize(value, &streamWithtype,
					&size, true);
		} else {
			const T *dataWithType = (const T *) readonlyData;
			*value = dataWithType[row * columns + column];
			return HasReturnvaluesIF::RETURN_OK;
		}
	}

	template<typename T>
	void set(T *data, uint8_t rows, uint8_t columns) {
		this->data = data;
		this->readonlyData = data;
		this->type = PodTypeConversion<T>::type;
		this->rows = rows;
		this->columns = columns;
		this->pointsToStream = false;
	}

	template<typename T>
	void set(const T *readonlyData, uint8_t rows, uint8_t columns) {
		this->data = NULL;
		this->readonlyData = readonlyData;
		this->type = PodTypeConversion<T>::type;
		this->rows = rows;
		this->columns = columns;
		this->pointsToStream = false;
	}

	template<typename T>
	void set(T& member) {
		this->set(&member, 1, 1);
	}

	template<typename T>
	void set(const T& readonlyMember) {
		this->set(&readonlyMember, 1, 1);
	}

	template<typename T>
	void setVector(T& member) {
		this->set(member, 1, sizeof(member)/sizeof(member[0]));
	}

	template<typename T>
	void setVector(const T& member) {
		this->set(member, 1, sizeof(member)/sizeof(member[0]));
	}
	template<typename T>
	void setMatrix(T& member) {
		this->set(member[0], sizeof(member)/sizeof(member[0]), sizeof(member[0])/sizeof(member[0][0]));
	}

	template<typename T>
	void setMatrix(const T& member) {
		this->set(member[0], sizeof(member)/sizeof(member[0]), sizeof(member[0])/sizeof(member[0][0]));
	}
	ReturnValue_t set(const uint8_t *stream, int32_t streamSize,
			const uint8_t **remainingStream = NULL, int32_t *remainingSize =
					NULL);

	ReturnValue_t copyFrom(const ParameterWrapper *from,
			uint16_t startWritingAtIndex);

private:
	bool pointsToStream;

	Type type;
	uint8_t rows;
	uint8_t columns;
	void *data;
	const void *readonlyData;

	template<typename T>
	ReturnValue_t serializeData(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const;

	template<typename T>
	ReturnValue_t deSerializeData(uint8_t startingRow, uint8_t startingColumn,
			const void *from, uint8_t fromRows, uint8_t fromColumns);
};

#endif /* PARAMETERWRAPPER_H_ */
