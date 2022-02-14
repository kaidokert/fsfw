#ifndef FSFW_PUS_SERVICEPACKETS_SERVICE2PACKETS_H_
#define FSFW_PUS_SERVICEPACKETS_SERVICE2PACKETS_H_

#include "../../action/ActionMessage.h"
#include "../../objectmanager/SystemObjectIF.h"
#include "../../serialize/SerialLinkedListAdapter.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

/**
 * @brief Subservice 128
 * @ingroup spacepackets
 */
class RawCommand {  //!< [EXPORT] : [SUBSERVICE] 128
 public:
  RawCommand(const uint8_t* buffer, size_t size) {
    // Deserialize Adapter to get correct endianness
    SerializeAdapter::deSerialize(&objectId, &buffer, &size, SerializeIF::Endianness::BIG);
    commandBuffer = buffer;
    // size is decremented by AutoSerializeAdapter,
    // remaining size is data size
    dataSize = size;
  }
  object_id_t getObjectId() const { return objectId; }

  const uint8_t* getCommand() { return commandBuffer; }

  size_t getCommandSize() const { return dataSize; }

 private:
  object_id_t objectId = 0;
  const uint8_t* commandBuffer = nullptr;  //!< [EXPORT] : [MAXSIZE] 256 Bytes
  size_t dataSize = 0;                     //!< [EXPORT] : [IGNORE]
};

/**
 * @brief Subservice 129: Command packet to set wiretapping mode
 * @ingroup spacepackets
 */
class WiretappingToggle
    : public SerialLinkedListAdapter<SerializeIF> {  //!< [EXPORT] : [SUBSERVICE] 129
 public:
  static const size_t WIRETAPPING_COMMAND_SIZE = 5;
  WiretappingToggle() {
    setStart(&objectId);
    objectId.setNext(&wiretappingMode);
  }

  uint8_t getWiretappingMode() const { return wiretappingMode.entry; }

 private:
  SerializeElement<object_id_t> objectId;
  SerializeElement<uint8_t> wiretappingMode;  //!< [EXPORT] : [INPUT] Mode 0: OFF, Mode 1: RAW
};

/**
 * @brief Subservices 130 and 131: TM packets
 * @ingroup spacepackets
 */
class WiretappingPacket {  //!< [EXPORT] : [SUBSERVICE] 130, 131
 public:
  object_id_t objectId;  //!< [EXPORT] : [COMMENT] Object ID of source object
  const uint8_t* data;   //!< [EXPORT] : [MAXSIZE] Raw Command Max. Size
  WiretappingPacket(object_id_t objectId, const uint8_t* buffer)
      : objectId(objectId), data(buffer) {}
};

#endif /* FSFW_PUS_SERVICEPACKETS_SERVICE2PACKETS_H_ */
