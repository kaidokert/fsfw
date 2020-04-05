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
		size_t* size, const size_t max_size, bool bigEndian) const {
	ReturnValue_t result = SerializeAdapter<object_id_t>::serialize(&objectId,
			buffer, size, max_size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = SerializeAdapter<ActionId_t>::serialize(&actionId, buffer,
			size, max_size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return data->serialize(buffer, size, max_size, bigEndian);
}

size_t DeviceTmReportingWrapper::getSerializedSize() const {
	return sizeof(objectId) + sizeof(ActionId_t) + data->getSerializedSize();
}

ReturnValue_t DeviceTmReportingWrapper::deSerialize(const uint8_t** buffer,
		ssize_t* size, bool bigEndian) {
	ReturnValue_t result = SerializeAdapter<object_id_t>::deSerialize(&objectId,
			buffer, size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = SerializeAdapter<ActionId_t>::deSerialize(&actionId, buffer,
			size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return data->deSerialize(buffer, size, bigEndian);
}
