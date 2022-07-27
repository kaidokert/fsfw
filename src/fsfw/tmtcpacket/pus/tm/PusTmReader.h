#ifndef FSFW_TMTCPACKET_PUSTMREADER_H
#define FSFW_TMTCPACKET_PUSTMREADER_H

#include "fsfw/timemanager/TimeReaderIF.h"
#include "fsfw/tmtcpacket/ReadablePacketIF.h"
#include "fsfw/tmtcpacket/RedirectableDataPointerIF.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketReader.h"
#include "fsfw/tmtcpacket/pus/RawUserDataReaderIF.h"
#include "fsfw/tmtcpacket/pus/tm/PusTmIF.h"

class PusTmReader : public PusTmIF,
                    public RawUserDataReaderIF,
                    public ReadablePacketIF,
                    public RedirectableDataPointerIF {
 public:
  explicit PusTmReader(TimeReaderIF* timeReader);
  PusTmReader(const uint8_t* data, size_t size);
  PusTmReader(TimeReaderIF* timeReader, const uint8_t* data, size_t size);

  ReturnValue_t parseDataWithoutCrcCheck();
  ReturnValue_t parseDataWithCrcCheck();
  [[nodiscard]] const uint8_t* getFullData() const override;

  explicit operator bool() const;
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
