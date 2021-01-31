#ifndef FSFW_PUS_SERVICEPACKETS_SERVICE20PACKETS_H_
#define FSFW_PUS_SERVICEPACKETS_SERVICE20PACKETS_H_

#include <fsfw/parameters/HasParametersIF.h>
#include <fsfw/serialize/SerialBufferAdapter.h>
#include <fsfw/serialize/SerializeElement.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>

/**
 * @brief
 * @details
 * @author
 */
class ParameterCommand: public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 128, 129, 130
public:
    ParameterCommand(uint8_t* storePointer, size_t parameterDataLen):
        parameterBuffer(storePointer, parameterDataLen) {
        setLinks();
    }

    ParameterCommand(object_id_t objectId, ParameterId_t parameterId,
            const uint8_t* parameterBuffer, size_t parameterBufferSize):
                objectId(objectId), parameterId(parameterId),
                parameterBuffer(parameterBuffer, parameterBufferSize) {
        setLinks();
    }

    ParameterId_t getParameterId() const {
        return parameterId.entry;
    }

    const uint8_t* getParameterBuffer() {
        return parameterBuffer.entry.getConstBuffer();
    }

    size_t getParameterBufferLen() const {
        return parameterBuffer.getSerializedSize();
    }

    uint8_t getPtc() const {
        return ccsdsType.entry >> 8 & 0xff;
    }

    uint8_t getPfc() const {
        return ccsdsType.entry & 0xff;
    }

    uint8_t getRows() const {
        return rows.entry;
    }

    uint8_t getColumns() const {
        return columns.entry;
    }

private:
    void setLinks() {
        setStart(&objectId);
        objectId.setNext(&parameterId);
        parameterId.setNext(&ccsdsType);
        ccsdsType.setNext(&rows);
        rows.setNext(&columns);
        columns.setNext(&parameterBuffer);
    }

    SerializeElement<object_id_t> objectId = 0;
    SerializeElement<ParameterId_t> parameterId = 0;
    //! [EXPORT] : [COMMENT] Type consisting of one byte PTC and one byte PFC.
    SerializeElement<uint16_t> ccsdsType = 0;
    SerializeElement<uint8_t> rows = 0;
    SerializeElement<uint8_t> columns = 0;
    SerializeElement<SerialBufferAdapter<uint8_t>> parameterBuffer;
};

class ParameterLoadCommand: public ParameterCommand {
public:
    ParameterLoadCommand(uint8_t* parameterPacket, size_t parameterDataLen):
            ParameterCommand(parameterPacket, parameterDataLen) {}
};

class ParameterDumpReply: public ParameterCommand {
public:
    ParameterDumpReply(object_id_t objectId, ParameterId_t parameterId,
            const uint8_t* parameterBuffer, size_t parameterBufferSize):
            ParameterCommand(objectId, parameterId, parameterBuffer, parameterBufferSize) {}
};

#endif /* FSFW_PUS_SERVICEPACKETS_SERVICE20PACKETS_H_ */
