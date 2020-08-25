#include "../serialize/SerializeAdapter.h"
#include "DeviceTmReportingWrapper.h"
#include "../serialize/SerializeAdapter.h"

DeviceTmReportingWrapper::DeviceTmReportingWrapper(object_id_t objectId,
		ActionId_t actionId, SerializeIF* data) :
		objectId(objectId), actionId(actionId), data(data) {
}

DeviceTmReportingWrapper::~DeviceTmReportingWrapper() {

}

ReturnValue_t DeviceTmReportingWrapper::serialize(uint8_t** buffer,
		size_t* size, size_t maxSize, Endianness streamEndianness) const {
	ReturnValue_t result = SerializeAdapter::serialize(&objectId,
			buffer, size, maxSize, streamEndianness);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = SerializeAdapter::serialize(&actionId, buffer,
			size, maxSize, streamEndianness);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return data->serialize(buffer, size, maxSize, streamEndianness);
}

size_t DeviceTmReportingWrapper::getSerializedSize() const {
	return sizeof(objectId) + sizeof(ActionId_t) + data->getSerializedSize();
}

ReturnValue_t DeviceTmReportingWrapper::deSerialize(const uint8_t** buffer,
		size_t* size, Endianness streamEndianness) {
	ReturnValue_t result = SerializeAdapter::deSerialize(&objectId,
			buffer, size, streamEndianness);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = SerializeAdapter::deSerialize(&actionId, buffer,
			size, streamEndianness);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return data->deSerialize(buffer, size, streamEndianness);
}
