#ifndef FSFW_SRC_FSFW_CFDP_PDU_VARLENFIELD_H_
#define FSFW_SRC_FSFW_CFDP_PDU_VARLENFIELD_H_

#include "../definitions.h"
#include "fsfw/serialize/SerializeIF.h"
#include <cstddef>
#include <cstdint>

namespace cfdp {

class VarLenField: public SerializeIF {
public:
    union LengthFieldLen {
        uint8_t oneByte;
        uint16_t twoBytes;
        uint32_t fourBytes;
        uint64_t eightBytes;
    };

    VarLenField();
    VarLenField(cfdp::WidthInBytes width, size_t value);

    ReturnValue_t setValue(cfdp::WidthInBytes, size_t value);

    ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
            Endianness streamEndianness) const override;

    size_t getSerializedSize() const override;

    ReturnValue_t deSerialize(cfdp::WidthInBytes width, const uint8_t **buffer, size_t *size,
            Endianness streamEndianness);

    cfdp::WidthInBytes getWidth() const;
    size_t getValue() const;
private:
    ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override;

    cfdp::WidthInBytes width;
    LengthFieldLen value;
};

}

#endif /* FSFW_SRC_FSFW_CFDP_PDU_VARLENFIELD_H_ */
