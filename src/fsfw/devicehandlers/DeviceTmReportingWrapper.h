#ifndef FSFW_DEVICEHANDLERS_DEVICETMREPORTINGWRAPPER_H_
#define FSFW_DEVICEHANDLERS_DEVICETMREPORTINGWRAPPER_H_

#include "../action/HasActionsIF.h"
#include "../objectmanager/SystemObjectIF.h"
#include "../serialize/SerializeIF.h"

class DeviceTmReportingWrapper : public SerializeIF {
 public:
  DeviceTmReportingWrapper(object_id_t objectId, ActionId_t actionId, const SerializeIF& data);
  ~DeviceTmReportingWrapper() override;

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

  [[nodiscard]] size_t getSerializedSize() const override;

 private:
  object_id_t objectId;
  ActionId_t actionId;
  const SerializeIF& data;

  // Deserialization forbidden
  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override;
};

#endif /* FSFW_DEVICEHANDLERS_DEVICETMREPORTINGWRAPPER_H_ */
