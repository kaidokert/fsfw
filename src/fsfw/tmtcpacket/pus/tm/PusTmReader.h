#ifndef FSFW_TMTCPACKET_PUSTMREADER_H
#define FSFW_TMTCPACKET_PUSTMREADER_H

#include "fsfw/timemanager/TimeReaderIF.h"
#include "fsfw/tmtcpacket/ReadablePacketIF.h"
#include "fsfw/tmtcpacket/RedirectableDataPointerIF.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketReader.h"
#include "fsfw/tmtcpacket/pus/RawUserDataReaderIF.h"
#include "fsfw/tmtcpacket/pus/tm/PusTmIF.h"

/**
 * This object can be used to read existing PUS TM packets in raw byte format.
 * It is a zero-copy object, so reading a TM packet with will not copy anything.
 *
 * Please note that a parser function must be called after the constructor. This will also check
 * the packet for validity.
 *
 * There are two parser function, where one does not perform the CRC check. This is useful
 * if the CRC calculation will is performed in a separate step.
 * This object also requires an explicit time stamp reader to allow flexibility in the used
 * timestamp.
 */
class PusTmReader : public PusTmIF,
                    public RawUserDataReaderIF,
                    public ReadablePacketIF,
                    public RedirectableDataPointerIF {
 public:
  explicit PusTmReader(TimeReaderIF* timeReader);
  PusTmReader(const uint8_t* data, size_t size);
  PusTmReader(TimeReaderIF* timeReader, const uint8_t* data, size_t size);

  /**
   * No CRC check will be performed
   * @return
   */
  ReturnValue_t parseDataWithoutCrcCheck();
  /**
   * Performs a CRC check on the data as well
   * @return
   *   - HasReturnvaluesIF::RETURN_OK: Successfully parsed the packet
   *   - SerializeIF::STREAM_TOO_SHORT: Stream too short for detected packet size
   *   - PusIF::INVALID_CRC_16 on invalid CRC
   */
  ReturnValue_t parseDataWithCrcCheck();
  [[nodiscard]] const uint8_t* getFullData() const override;

  /**
   * Returns @isNull
   * @return
   */
  explicit operator bool() const;
  /**
   * No (valid) data was set yet or the parse function was not called yet.
   * @return
   */
  [[nodiscard]] bool isNull() const;
  void setTimeReader(TimeReaderIF* timeReader);
  TimeReaderIF* getTimeReader();
  ReturnValue_t setReadOnlyData(const uint8_t* data, size_t size);
  [[nodiscard]] uint16_t getPacketIdRaw() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrlRaw() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;
  [[nodiscard]] uint8_t getPusVersion() const override;
  [[nodiscard]] uint8_t getService() const override;
  [[nodiscard]] uint8_t getSubService() const override;
  [[nodiscard]] const uint8_t* getUserData() const override;
  [[nodiscard]] size_t getUserDataLen() const override;
  uint8_t getScTimeRefStatus() override;
  uint16_t getMessageTypeCounter() override;
  uint16_t getDestId() override;

 protected:
  ecss::PusPointers pointers{};
  SpacePacketReader spReader{};
  size_t sourceDataLen = 0;
  TimeReaderIF* timeReader{};
  ReturnValue_t setData(uint8_t* dataPtr, size_t size, void* args) override;
  ReturnValue_t parseData(bool crcCheck);

 private:
};

#endif  // FSFW_TMTCPACKET_PUSTMREADER_H
