#ifndef DEVICETMREPORTINGWRAPPER_H_
#define DEVICETMREPORTINGWRAPPER_H_

#include "../action/HasActionsIF.h"
#include "../objectmanager/SystemObjectIF.h"
#include "../serialize/SerializeIF.h"

class DeviceTmReportingWrapper: public SerializeIF {
public:
	DeviceTmReportingWrapper(object_id_t objectId, ActionId_t actionId,
			SerializeIF *data);
	virtual ~DeviceTmReportingWrapper();

	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			size_t maxSize, Endianness streamEndianness) const override;

	virtual size_t getSerializedSize() const override;

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
			Endianness streamEndianness) override;
private:
	object_id_t objectId;
	ActionId_t actionId;
	SerializeIF *data;
};

#endif /* DEVICETMREPORTINGWRAPPER_H_ */
