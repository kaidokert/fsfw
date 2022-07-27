#ifndef TMTCPACKET_PUS_TCPACKETBASE_H_
#define TMTCPACKET_PUS_TCPACKETBASE_H_

#include <cstddef>

#include "PusTcIF.h"
#include "fsfw/tmtcpacket/RedirectableDataPointerIF.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketReader.h"
#include "fsfw/tmtcpacket/pus/RawUserDataReaderIF.h"
#include "fsfw/tmtcpacket/pus/defs.h"

/**
 * This class is the basic reader class to read ECSS PUS C Telecommand packets.
 *
 *  1. Implements the @SpacePacketIF to provide accessor methods for the contained space packet.
 *  2. Implements the @PusTcIF to provide accessor methods for generic PUS C fields
 *
 * This is a zero-copy reader class. It does not contain the packet data itself but a pointer to
 * the data. Calling any accessor methods without pointing the object to valid data  first will
 * cause undefined behaviour.
 * @ingroup tmtcpackets
 */
class PusTcReader : public PusTcIF,
                    public RawUserDataReaderIF,
                    public ReadablePacketIF,
                    public RedirectableDataPointerIF {
 public:
  PusTcReader() = default;
  /**
   * This is the default constructor.
   * It sets its internal data pointer to the address passed and also
   * forwards the data pointer to the parent SpacePacketBase class.
   * @param setData	The position where the packet data lies.
   */
  PusTcReader(const uint8_t* setData, size_t size);
  ~PusTcReader() override;

  explicit operator bool() const;
  [[nodiscard]] bool isNull() const;
  ReturnValue_t parseDataWithCrcCheck();
  ReturnValue_t parseDataWithoutCrcCheck();

  /**
   * This is a debugging helper method that prints the whole packet content
   * to the screen.
   */
  // void print();
  [[nodiscard]] uint16_t getPacketIdRaw() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrlRaw() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;
  [[nodiscard]] uint8_t getPusVersion() const override;
  [[nodiscard]] uint8_t getAcknowledgeFlags() const override;
  [[nodiscard]] uint8_t getService() const override;
  [[nodiscard]] uint8_t getSubService() const override;
  [[nodiscard]] uint16_t getSourceId() const override;
  [[nodiscard]] uint16_t getErrorControl() const;
  const uint8_t* getFullData() const override;

  ReturnValue_t setReadOnlyData(const uint8_t* data, size_t size);
  [[nodiscard]] const uint8_t* getUserData() const override;
  [[nodiscard]] size_t getUserDataLen() const override;

 protected:
  /**
   * With this method, the packet data pointer can be redirected to another
   * location.
   * This call overwrites the parent's setData method to set both its
   * @c tc_data pointer and the parent's @c data pointer.
   *
   * @param p_data    A pointer to another PUS Telecommand Packet.
   */
  ReturnValue_t setData(uint8_t* pData, size_t size, void* args) override;
  ReturnValue_t parseData(bool withCrc);

  SpacePacketReader spReader;
  ecss::PusPointers pointers{};
  size_t appDataSize = 0;
};

#endif /* TMTCPACKET_PUS_TCPACKETBASE_H_ */
