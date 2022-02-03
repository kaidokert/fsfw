#ifndef FSFW_PUS_SERVICEPACKETS_SERVICE8PACKETS_H_
#define FSFW_PUS_SERVICEPACKETS_SERVICE8PACKETS_H_

#include "../../action/ActionMessage.h"
#include "../../objectmanager/SystemObjectIF.h"
#include "../../serialize/SerialBufferAdapter.h"
#include "../../serialize/SerialFixedArrayListAdapter.h"
#include "../../serialize/SerialLinkedListAdapter.h"
#include "../../serialize/SerializeElement.h"

/**
 * @brief Subservice 128
 * @ingroup spacepackets
 */
class DirectCommand
    : public SerialLinkedListAdapter<SerializeIF> {  //!< [EXPORT] : [SUBSERVICE] 128
 public:
  DirectCommand(const uint8_t* tcData, size_t size) {
    SerializeAdapter::deSerialize(&objectId, &tcData, &size, SerializeIF::Endianness::BIG);
    SerializeAdapter::deSerialize(&actionId, &tcData, &size, SerializeIF::Endianness::BIG);
    parameterBuffer = tcData;
    parametersSize = size;
  }

  ActionId_t getActionId() const { return actionId; }

  object_id_t getObjectId() const { return objectId; }

  const uint8_t* getParameters() { return parameterBuffer; }

  uint32_t getParametersSize() const { return parametersSize; }

 private:
  DirectCommand(const DirectCommand& command);
  object_id_t objectId = 0;
  ActionId_t actionId = 0;
  uint32_t parametersSize;         //!< [EXPORT] : [IGNORE]
  const uint8_t* parameterBuffer;  //!< [EXPORT] : [MAXSIZE] 65535 Bytes
};

/**
 * @brief Subservice 130
 * Data reply (subservice 130) consists of
 *   1. Target Object ID
 *   2. Action ID
 *   3. Data
 * @ingroup spacepackets
 */
class DataReply : public SerialLinkedListAdapter<SerializeIF> {  //!< [EXPORT] : [SUBSERVICE] 130
 public:
  typedef uint16_t typeOfMaxDataSize;
  static const uint16_t MAX_DATA_LENGTH = sizeof(typeOfMaxDataSize);
  DataReply(object_id_t objectId_, ActionId_t actionId_, const uint8_t* replyDataBuffer_ = NULL,
            uint16_t replyDataSize_ = 0)
      : objectId(objectId_), actionId(actionId_), replyData(replyDataBuffer_, replyDataSize_) {
    setLinks();
  }

 private:
  DataReply(const DataReply& reply);
  void setLinks() {
    setStart(&objectId);
    objectId.setNext(&actionId);
    actionId.setNext(&replyData);
  }
  SerializeElement<object_id_t> objectId;
  SerializeElement<ActionId_t> actionId;
  SerializeElement<SerialBufferAdapter<uint16_t>> replyData;
};

/**
 * @brief Subservice 132
 * @details
 * Not used yet. Telecommand Verification takes care of this.
 * @ingroup spacepackets
 */
class DirectReply : public SerialLinkedListAdapter<SerializeIF> {  //!< [EXPORT] : [SUBSERVICE] 132
 public:
  typedef uint16_t typeOfMaxDataSize;
  static const uint16_t MAX_DATA_LENGTH = sizeof(typeOfMaxDataSize);

  DirectReply(object_id_t objectId_, ActionId_t actionId_, ReturnValue_t returnCode_,
              bool isStep_ = false, uint8_t step_ = 0)
      : isStep(isStep_),
        objectId(objectId_),
        actionId(actionId_),
        returnCode(returnCode_),
        step(step_) {
    setLinks();
  }

 private:
  void setLinks() {
    setStart(&objectId);
    objectId.setNext(&actionId);
    actionId.setNext(&returnCode);
    if (isStep) {
      returnCode.setNext(&step);
    }
  }

  bool isStep;                                 //!< [EXPORT] : [IGNORE]
  SerializeElement<object_id_t> objectId;      //!< [EXPORT] : [IGNORE]
  SerializeElement<ActionId_t> actionId;       //!< [EXPORT] : [IGNORE]
  SerializeElement<ReturnValue_t> returnCode;  //!< [EXPORT] : [IGNORE]
  SerializeElement<uint8_t> step;              //!< [EXPORT] : [OPTIONAL] [IGNORE]
};

#endif /* FSFW_PUS_SERVICEPACKETS_SERVICE8PACKETS_H_ */
