#include <framework/serialize/SerializeAdapter.h>
#include <framework/devicehandlers/DeviceTmReportingWrapper.h>
#include <framework/serialize/SerializeAdapter.h>

DeviceTmReportingWrapper::DeviceTmReportingWrapper(object_id_t objectId,
		ActionId_t actionId, SerializeIF* data) :
		objectId(objectId), actionId(actionId), data(data) {
}

DeviceTmReportingWrapper::~DeviceTmReportingWrapper() {

}

ReturnValue_t DeviceTmReportingWrapper::serialize(uint8_t** buffer,
		uint32_t* size, const uint32_t max_size, bool bigEndian) const {
	ReturnValue_t result = SerializeAdapter::serialize(&objectId,
			buffer, size, max_size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = SerializeAdapter::serialize(&actionId, buffer,
			size, max_size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return data->serialize(buffer, size, max_size, bigEndian);
}

uint32_t DeviceTmReportingWrapper::getSerializedSize() const {
	return sizeof(objectId) + sizeof(ActionId_t) + data->getSerializedSize();
}

ReturnValue_t DeviceTmReportingWrapper::deSerialize(const uint8_t** buffer,
		int32_t* size, bool bigEndian) {
	ReturnValue_t result = SerializeAdapter::deSerialize(&objectId,
			buffer, size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = SerializeAdapter::deSerialize(&actionId, buffer,
			size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return data->deSerialize(buffer, size, bigEndian);
}
