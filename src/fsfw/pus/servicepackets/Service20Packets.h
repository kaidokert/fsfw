#ifndef FSFW_PUS_SERVICEPACKETS_SERVICE20PACKETS_H_
#define FSFW_PUS_SERVICEPACKETS_SERVICE20PACKETS_H_

#include <FSFWConfig.h>
#include <fsfw/parameters/HasParametersIF.h>
#include <fsfw/serialize/SerialBufferAdapter.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <fsfw/serialize/SerializeElement.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

/**
 * @brief   This class encapsulates the packets sent to the PUS service 20 or sent by the
 *          PUS service 20
 * @details
 * This command can be used to handle both load and dump commands as well.
 * @author
 */
class ParameterCommand
    : public SerialLinkedListAdapter<SerializeIF> {  //!< [EXPORT] : [SUBSERVICE] 128, 129, 130
 public:
  /**
   * This constructor is used for load replies. The data is expected in the correct formast
   * in the store pointer.
   * @param storePointer
   * @param parameterDataLen
   */
  ParameterCommand(uint8_t* storePointer, size_t parameterDataLen)
      : parameterBuffer(storePointer, parameterDataLen) {
#if FSFW_VERBOSE_LEVEL >= 1
    if (parameterDataLen == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "ParameterCommand: Parameter data length is 0" << std::endl;
#else
      sif::printWarning("ParameterCommand: Parameter data length is 0!\n");
#endif
    }
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    setLoadLinks();
  }

  /**
   * This constructor is used for dump replies. It is assumed the 4 byte parameter
   * information field is located inside the parameter buffer.
   * @param objectId
   * @param parameterId
   * @param parameterBuffer
   * @param parameterBufferSize
   */
  ParameterCommand(object_id_t objectId, ParameterId_t parameterId, const uint8_t* parameterBuffer,
                   size_t parameterBufferSize)
      : objectId(objectId),
        parameterId(parameterId),
        parameterBuffer(parameterBuffer, parameterBufferSize) {
    setDumpReplyLinks();
  }

  ParameterId_t getParameterId() const { return parameterId.entry; }

  const uint8_t* getParameterBuffer() { return parameterBuffer.entry.getConstBuffer(); }

  size_t getParameterBufferLen() const { return parameterBuffer.getSerializedSize(); }

  uint8_t getDomainId() const { return (parameterId.entry >> 24) & 0xff; }

  uint8_t getUniqueId() const { return (parameterId.entry >> 16) & 0xff; }

  uint16_t getLinearIndex() const { return parameterId.entry & 0xffff; }

  uint8_t getPtc() const { return ccsdsType.entry >> 8 & 0xff; }

  uint8_t getPfc() const { return ccsdsType.entry & 0xff; }

  uint8_t getRows() const { return rows.entry; }

  uint8_t getColumns() const { return columns.entry; }

 private:
  void setLoadLinks() {
    setStart(&objectId);
    objectId.setNext(&parameterId);
    parameterId.setNext(&ccsdsType);
    ccsdsType.setNext(&rows);
    rows.setNext(&columns);
    columns.setNext(&parameterBuffer);
  }

  void setDumpReplyLinks() {
    /* For a dump reply, the parameter information is contained in the parameter buffer
    with the actual parameters */
    setStart(&objectId);
    objectId.setNext(&parameterId);
    parameterId.setNext(&parameterBuffer);
  }

  void setDumpRequestLinks() {
    setStart(&objectId);
    objectId.setNext(&parameterId);
  }

  SerializeElement<object_id_t> objectId = 0;
  SerializeElement<ParameterId_t> parameterId = 0;
  //! [EXPORT] : [COMMENT] Type consisting of one byte PTC and one byte PFC.
  SerializeElement<uint16_t> ccsdsType = 0;
  SerializeElement<uint8_t> rows = 0;
  SerializeElement<uint8_t> columns = 0;
  SerializeElement<SerialBufferAdapter<uint8_t>> parameterBuffer;
};

class ParameterLoadCommand : public ParameterCommand {
 public:
  ParameterLoadCommand(uint8_t* parameterPacket, size_t parameterDataLen)
      : ParameterCommand(parameterPacket, parameterDataLen) {}
};

class ParameterDumpReply : public ParameterCommand {
 public:
  ParameterDumpReply(object_id_t objectId, ParameterId_t parameterId,
                     const uint8_t* parameterBuffer, size_t parameterBufferSize)
      : ParameterCommand(objectId, parameterId, parameterBuffer, parameterBufferSize) {}
};

#endif /* FSFW_PUS_SERVICEPACKETS_SERVICE20PACKETS_H_ */
