#include "SimpleRingBuffer.h"
#include <cstring>

SimpleRingBuffer::SimpleRingBuffer(const size_t size, bool overwriteOld,
		size_t maxExcessBytes) :
		RingBufferBase<>(0, size, overwriteOld),
		maxExcessBytes(maxExcessBytes) {
	if(maxExcessBytes > size) {
		this->maxExcessBytes = size;
	}
	else {
		this->maxExcessBytes = maxExcessBytes;
	}
	buffer = new uint8_t[size + maxExcessBytes];
}

SimpleRingBuffer::SimpleRingBuffer(uint8_t *buffer, const size_t size,
		bool overwriteOld, size_t maxExcessBytes):
        RingBufferBase<>(0, size, overwriteOld), buffer(buffer) {
	if(maxExcessBytes > size) {
		this->maxExcessBytes = size;
	}
	else {
		this->maxExcessBytes = maxExcessBytes;
	}
}

SimpleRingBuffer::~SimpleRingBuffer() {
	delete[] buffer;
}

ReturnValue_t SimpleRingBuffer::getFreeElement(uint8_t **writePointer,
        size_t amount) {
    if (availableWriteSpace() >= amount or overwriteOld) {
        size_t amountTillWrap = writeTillWrap();
        if (amountTillWrap < amount) {
            if((amount - amountTillWrap + excessBytes) > maxExcessBytes) {
                return HasReturnvaluesIF::RETURN_FAILED;
            }
            excessBytes = amount - amountTillWrap;
        }
        *writePointer = &buffer[write];
        return HasReturnvaluesIF::RETURN_OK;
    }
    else {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}

void SimpleRingBuffer::confirmBytesWritten(size_t amount) {
	if(getExcessBytes() > 0) {
		moveExcessBytesToStart();
	}
	incrementWrite(amount);

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
	size_t availableData = getAvailableReadData(READ_PTR);
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

size_t SimpleRingBuffer::getExcessBytes() const {
    return excessBytes;
}

void SimpleRingBuffer::moveExcessBytesToStart() {
    if(excessBytes > 0) {
        std::memcpy(buffer, &buffer[size], excessBytes);
        excessBytes = 0;
    }
}

ReturnValue_t SimpleRingBuffer::deleteData(size_t amount,
		bool deleteRemaining, size_t* trueAmount) {
	size_t availableData = getAvailableReadData(READ_PTR);
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
