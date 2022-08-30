#ifndef FSFW_DEVICEHANDLERS_DEVICETMREPORTINGWRAPPER_H_
#define FSFW_DEVICEHANDLERS_DEVICETMREPORTINGWRAPPER_H_

#include "fsfw/action/HasActionsIF.h"
#include "fsfw/objectmanager/SystemObjectIF.h"
#include "fsfw/serialize/SerializeIF.h"
#include "fsfw/util/dataWrapper.h"

class DeviceTmReportingWrapper : public SerializeIF {
 public:
  DeviceTmReportingWrapper(object_id_t objectId, ActionId_t actionId, util::DataWrapper data);
  ~DeviceTmReportingWrapper() override;

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

  [[nodiscard]] size_t getSerializedSize() const override;

 private:
  object_id_t objectId;
  ActionId_t actionId;
  util::DataWrapper dataWrapper;

  // Deserialization forbidden
  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override;
};

#endif /* FSFW_DEVICEHANDLERS_DEVICETMREPORTINGWRAPPER_H_ */
