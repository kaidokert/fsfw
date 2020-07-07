#include <framework/container/SimpleRingBuffer.h>
#include <cstring>

SimpleRingBuffer::SimpleRingBuffer(const size_t size, bool overwriteOld) :
		RingBufferBase<>(0, size, overwriteOld) {
	buffer = new uint8_t[size];
}

SimpleRingBuffer::SimpleRingBuffer(uint8_t *buffer, const size_t size,
        bool overwriteOld):
        RingBufferBase<>(0, size, overwriteOld), buffer(buffer) {}


SimpleRingBuffer::~SimpleRingBuffer() {
	delete[] buffer;
}

ReturnValue_t SimpleRingBuffer::writeData(const uint8_t* data,
		uint32_t amount) {
	if (availableWriteSpace() >= amount or overwriteOld) {
		uint32_t amountTillWrap = writeTillWrap();
		if (amountTillWrap >= amount) {
			memcpy(&buffer[write], data, amount);
		} else {
			memcpy(&buffer[write], data, amountTillWrap);
			memcpy(buffer, data + amountTillWrap, amount - amountTillWrap);
		}
		incrementWrite(amount);
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t SimpleRingBuffer::readData(uint8_t* data, uint32_t amount,
		bool readRemaining, uint32_t* trueAmount) {
	uint32_t availableData = availableReadData(READ_PTR);
	uint32_t amountTillWrap = readTillWrap(READ_PTR);
	if (availableData < amount) {
		if (readRemaining) {
			amount = availableData;
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	if (trueAmount != nullptr) {
		*trueAmount = amount;
	}
	if (amountTillWrap >= amount) {
		memcpy(data, &buffer[read[READ_PTR]], amount);
	} else {
		memcpy(data, &buffer[read[READ_PTR]], amountTillWrap);
		memcpy(data + amountTillWrap, buffer, amount - amountTillWrap);
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SimpleRingBuffer::deleteData(uint32_t amount,
		bool deleteRemaining, uint32_t* trueAmount) {
	uint32_t availableData = availableReadData(READ_PTR);
	if (availableData < amount) {
		if (deleteRemaining) {
			amount = availableData;
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	if (trueAmount != nullptr) {
		*trueAmount = amount;
	}
	incrementRead(amount, READ_PTR);
	return HasReturnvaluesIF::RETURN_OK;
}

