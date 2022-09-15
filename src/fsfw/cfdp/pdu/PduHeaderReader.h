#ifndef FSFW_SRC_FSFW_CFDP_PDU_HEADERDESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_HEADERDESERIALIZER_H_

#include <cstddef>
#include <cstdint>

#include "PduConfig.h"
#include "PduHeaderIF.h"
#include "fsfw/serialize/SerializeIF.h"
#include "fsfw/tmtcpacket/RedirectableDataPointerIF.h"

struct PduHeaderFixedStruct {
  uint8_t firstByte;
  uint8_t pduDataFieldLenH;
  uint8_t pduDataFieldLenL;
  uint8_t fourthByte;
  uint8_t variableFieldsStart;
};

/**
 * @brief   This class is used to deserialize a PDU header from raw memory.
 * @details
 * This is a zero-copy implementation and #parseData needs to be called to ensure the data is
 * valid.
 */
class PduHeaderReader : public RedirectableDataPointerIF, public PduHeaderIF {
 public:
  PduHeaderReader() = default;
  /**
   * Initialize a PDU header from raw data. This is a zero-copy implementation and #parseData
   * needs to be called to ensure the data is valid
   * @param pduBuf
   * @param maxSize
   */
  PduHeaderReader(const uint8_t* pduBuf, size_t maxSize);

  /**
   * This needs to be called before accessing the PDU fields to avoid segmentation faults.
   * @return
   *  - returnvalue::OK on parse success
   *  - returnvalue::FAILED Invalid raw data
   *  - SerializeIF::BUFFER_TOO_SHORT if buffer is shorter than expected
   */
  virtual ReturnValue_t parseData();
  explicit operator bool() const;
  [[nodiscard]] bool isNull() const;

  /**
   * Fill the provided PDU configuration from the fields detected by this reader.
   * @param cfg
   */
  void fillConfig(PduConfig& cfg) const;

  [[nodiscard]] virtual size_t getHeaderSize() const;

  [[nodiscard]] size_t getPduDataFieldLen() const override;
  [[nodiscard]] size_t getWholePduSize() const override;

  [[nodiscard]] cfdp::PduType getPduType() const override;
  [[nodiscard]] cfdp::Direction getDirection() const override;
  [[nodiscard]] cfdp::TransmissionMode getTransmissionMode() const override;
  [[nodiscard]] bool getCrcFlag() const override;
  [[nodiscard]] bool getLargeFileFlag() const override;
  [[nodiscard]] cfdp::SegmentationControl getSegmentationControl() const override;
  [[nodiscard]] cfdp::WidthInBytes getLenEntityIds() const override;
  [[nodiscard]] cfdp::WidthInBytes getLenSeqNum() const override;
  [[nodiscard]] cfdp::SegmentMetadataFlag getSegmentMetadataFlag() const override;
  [[nodiscard]] bool hasSegmentMetadataFlag() const override;

  void getSourceId(cfdp::EntityId& sourceId) const override;
  void getDestId(cfdp::EntityId& destId) const override;
  void getTransactionSeqNum(cfdp::TransactionSeqNum& seqNum) const override;

  ReturnValue_t deserResult = returnvalue::OK;

  [[nodiscard]] size_t getMaxSize() const;
  [[nodiscard]] const uint8_t* getPduDataField() const;

  /**
   * Can also be used to reset the pointer to a nullptr, but the getter functions will not
   * perform nullptr checks!
   * @param dataPtr
   * @param maxSize
   * @param args
   * @return
   */
  ReturnValue_t setReadOnlyData(const uint8_t* dataPtr, size_t maxSize);

 protected:
  struct Pointers {
    PduHeaderFixedStruct* fixedHeader = nullptr;
    const uint8_t* dataFieldStart = nullptr;
    const uint8_t* rawPtr = nullptr;
  };

  Pointers pointers;
  size_t maxSize = 0;

 private:
  /**
   * This is a reader class and setting mutable data is forbidden. Use @setReadOnlyData instead.
   * @param dataPtr
   * @param maxSize
   * @param args
   * @return
   */
  ReturnValue_t setData(uint8_t* dataPtr, size_t maxSize, void* args) override;
  void assignVarLenField(cfdp::VarLenField* field, cfdp::WidthInBytes width, void* sourcePtr) const;
  void* sourceIdRaw = nullptr;
  void* seqNumRaw = nullptr;
  void* destIdRaw = nullptr;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_HEADERDESERIALIZER_H_ */
