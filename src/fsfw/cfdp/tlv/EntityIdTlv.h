#ifndef FSFW_SRC_FSFW_CFDP_ENTITYIDTLV_H_
#define FSFW_SRC_FSFW_CFDP_ENTITYIDTLV_H_

#include "TlvIF.h"
#include "fsfw/cfdp/pdu/PduConfig.h"
#include "fsfw/cfdp/tlv/Tlv.h"

class EntityIdTlv : public TlvIF {
 public:
  EntityIdTlv(cfdp::EntityId& entityId);
  virtual ~EntityIdTlv();

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

  size_t getSerializedSize() const override;

  /**
   * Deserialize an entity ID from a raw TLV object
   * @param tlv
   * @param endianness
   * @return
   */
  ReturnValue_t deSerialize(cfdp::Tlv& tlv, Endianness endianness);

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override;

  uint8_t getLengthField() const override;
  cfdp::TlvType getType() const override;

  cfdp::EntityId& getEntityId();

 private:
  cfdp::EntityId& entityId;
};

#endif /* FSFW_SRC_FSFW_CFDP_ENTITYIDTLV_H_ */
