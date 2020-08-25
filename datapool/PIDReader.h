#ifndef PIDREADER_H_
#define PIDREADER_H_
#include "DataPool.h"
#include "DataSetIF.h"
#include "PoolEntry.h"
#include "PoolVariableIF.h"
#include "../serialize/SerializeAdapter.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

template<typename U, uint8_t n_var> class PIDReaderList;

template<typename T>
class PIDReader: public PoolVariableIF {
	template<typename U, uint8_t n_var> friend class PIDReaderList;
protected:
	uint32_t parameterId;
	uint8_t valid;
	ReturnValue_t read() {
		uint8_t arrayIndex = DataPool::PIDToArrayIndex(parameterId);
		PoolEntry<T> *read_out = ::dataPool.getData<T>(
				DataPool::PIDToDataPoolId(parameterId), arrayIndex);
		if (read_out != NULL) {
			valid = read_out->valid;
			value = read_out->address[arrayIndex];
			return HasReturnvaluesIF::RETURN_OK;
		} else {
			value = 0;
			valid = false;
			sif::error << "PIDReader: read of PID 0x" << std::hex << parameterId
					<< std::dec << " failed." << std::endl;
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	/**
	 * Never commit, is read-only.
	 * Reason is the possibility to access a single DP vector element, but if we commit,
	 * we set validity of the whole vector.
	 */
	ReturnValue_t commit() {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	/**
	 * Empty ctor for List initialization
	 */
	PIDReader() :
			parameterId(PoolVariableIF::NO_PARAMETER), valid(
					PoolVariableIF::INVALID), value(0) {

	}
public:
	/**
	 * \brief	This is the local copy of the data pool entry.
	 */
	T value;
	/**
	 * \brief	In the constructor, the variable can register itself in a DataSet (if not NULL is
	 * 			passed).
	 * \details	It DOES NOT fetch the current value from the data pool, but sets the value
	 * 			attribute to default (0). The value is fetched within the read() operation.
	 * \param set_id	This is the id in the global data pool this instance of the access class
	 * 					corresponds to.
	 * \param dataSet	The data set in which the variable shall register itself. If NULL,
	 * 					the variable is not registered.
	 * \param setWritable If this flag is set to true, changes in the value attribute can be
	 * 					written back to the data pool, otherwise not.
	 */
	PIDReader(uint32_t setParameterId, DataSetIF *dataSet) :
			parameterId(setParameterId), valid(PoolVariableIF::INVALID), value(
					0) {
		if (dataSet != NULL) {
			dataSet->registerVariable(this);
		}
	}

	/**
	 * Copy ctor to copy classes containing Pool Variables.
	 */
	PIDReader(const PIDReader &rhs) :
			parameterId(rhs.parameterId), valid(rhs.valid), value(rhs.value) {
	}

	/**
	 * \brief	The classes destructor is empty.
	 */
	~PIDReader() {

	}
	/**
	 * \brief	This operation returns the data pool id of the variable.
	 */
	uint32_t getDataPoolId() const {
		return DataPool::PIDToDataPoolId(parameterId);
	}
	uint32_t getParameterId() const {
		return parameterId;
	}
	/**
	 * This method returns if the variable is write-only, read-write or read-only.
	 */
	ReadWriteMode_t getReadWriteMode() const {
		return VAR_READ;
	}
	/**
	 * \brief	With this call, the valid information of the variable is returned.
	 */
	bool isValid() const {
		if (valid)
			return true;
		else
			return false;
	}

	uint8_t getValid() {
		return valid;
	}

	void setValid(uint8_t valid) {
		this->valid = valid;
	}

	operator T() {
		return value;
	}

	PIDReader<T>& operator=(T newValue) {
		value = newValue;
		return *this;
	}

	virtual ReturnValue_t serialize(uint8_t **buffer, size_t *size,
			size_t maxSize, Endianness streamEndianness) const override {
		return SerializeAdapter::serialize(&value, buffer, size, maxSize,
				streamEndianness);
	}

	virtual size_t getSerializedSize() const override {
		return SerializeAdapter::getSerializedSize(&value);
	}

	virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
			Endianness streamEndianness) override {
		return SerializeAdapter::deSerialize(&value, buffer, size,
				streamEndianness);
	}
};

#endif /* PIDREADER_H_ */
