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
class HeaderReader : public RedirectableDataPointerIF, public PduHeaderIF {
 public:
  /**
   * Initialize a PDU header from raw data. This is a zero-copy implementation and #parseData
   * needs to be called to ensure the data is valid
   * @param pduBuf
   * @param maxSize
   */
  HeaderReader(const uint8_t* pduBuf, size_t maxSize);

  /**
   * This needs to be called before accessing the PDU fields to avoid segmentation faults.
   * @return
   *  - RETURN_OK on parse success
   *  - RETURN_FAILED Invalid raw data
   *  - SerializeIF::BUFFER_TOO_SHORT if buffer is shorter than expected
   */
  virtual ReturnValue_t parseData();
  [[nodiscard]] size_t getHeaderSize() const;

  [[nodiscard]] size_t getPduDataFieldLen() const override;
  [[nodiscard]] size_t getWholePduSize() const override;

  [[nodiscard]] cfdp::PduType getPduType() const override;
  [[nodiscard]] cfdp::Direction getDirection() const override;
  [[nodiscard]] cfdp::TransmissionModes getTransmissionMode() const override;
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

  ReturnValue_t deserResult = HasReturnvaluesIF::RETURN_OK;

  [[nodiscard]] size_t getMaxSize() const;

  /**
   * Can also be used to reset the pointer to a nullptr, but the getter functions will not
   * perform nullptr checks!
   * @param dataPtr
   * @param maxSize
   * @param args
   * @return
   */
  ReturnValue_t setData(const uint8_t* dataPtr, size_t maxSize);

 protected:
  PduHeaderFixedStruct* fixedHeader = nullptr;
  const uint8_t* rawPtr = nullptr;
  size_t maxSize = 0;

 private:
  /**
   * Can also be used to reset the pointer to a nullptr, but the getter functions will not
   * perform nullptr checks!
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
