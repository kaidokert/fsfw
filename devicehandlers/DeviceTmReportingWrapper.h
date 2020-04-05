#ifndef DEVICETMREPORTINGWRAPPER_H_
#define DEVICETMREPORTINGWRAPPER_H_

#include <framework/action/HasActionsIF.h>
#include <framework/objectmanager/SystemObjectIF.h>
#include <framework/serialize/SerializeIF.h>

class DeviceTmReportingWrapper: public SerializeIF {
public:
	DeviceTmReportingWrapper(object_id_t objectId, ActionId_t actionId,
			SerializeIF *data);
	virtual ~DeviceTmReportingWrapper();

	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			const size_t max_size, bool bigEndian) const;

	virtual uint32_t getSerializedSize() const;

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian);
private:
	object_id_t objectId;
	ActionId_t actionId;
	SerializeIF *data;
};

#endif /* DEVICETMREPORTINGWRAPPER_H_ */
