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
		size_t amount) {
	if (availableWriteSpace() >= amount or overwriteOld) {
		size_t amountTillWrap = writeTillWrap();
		if (amountTillWrap >= amount) {
			// remaining size in buffer is sufficient to fit full amount.
			memcpy(&buffer[write], data, amount);
		}
		else {
			memcpy(&buffer[write], data, amountTillWrap);
			memcpy(buffer, data + amountTillWrap, amount - amountTillWrap);
		}
		incrementWrite(amount);
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t SimpleRingBuffer::readData(uint8_t* data, size_t amount,
		bool incrementReadPtr, bool readRemaining, size_t* trueAmount) {
	size_t availableData = availableReadData(READ_PTR);
	size_t amountTillWrap = readTillWrap(READ_PTR);
	if (availableData < amount) {
		if (readRemaining) {
			// more data available than amount specified.
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

	if(incrementReadPtr) {
		deleteData(amount, readRemaining);
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SimpleRingBuffer::deleteData(size_t amount,
		bool deleteRemaining, size_t* trueAmount) {
	size_t availableData = availableReadData(READ_PTR);
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

