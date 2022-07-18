#ifndef TMTCPACKET_PUS_TCPACKETBASE_H_
#define TMTCPACKET_PUS_TCPACKETBASE_H_

#include <cstddef>

#include "PusTcIF.h"
#include "fsfw/tmtcpacket/RedirectableDataPointerIF.h"
#include "fsfw/tmtcpacket/SpacePacketReader.h"

/**
 * This class is the basic data handler for any ECSS PUS Telecommand packet.
 *
 * In addition to #SpacePacketBase, the class provides methods to handle
 * the standardized entries of the PUS TC Packet Data Field Header.
 * It does not contain the packet data itself but a pointer to the
 * data must be set on instantiation. An invalid pointer may cause
 * damage, as no getter method checks data validity. Anyway, a NULL
 * check can be performed by making use of the getWholeData method.
 * @ingroup tmtcpackets
 */
class PusTcReader : public PusTcIF, public RedirectableDataPointerIF {
  friend class TcPacketStoredBase;

 public:
  /**
   * This is the default constructor.
   * It sets its internal data pointer to the address passed and also
   * forwards the data pointer to the parent SpacePacketBase class.
   * @param setData	The position where the packet data lies.
   */
  explicit PusTcReader(const uint8_t* setData, size_t size);

  ReturnValue_t parseData();
  /**
   * This is the empty default destructor.
   */
  ~PusTcReader() override;

  /**
   * This is a debugging helper method that prints the whole packet content
   * to the screen.
   */
  void print();
  [[nodiscard]] uint16_t getPacketId() const override;
  [[nodiscard]] uint16_t getPacketSeqCtrl() const override;
  [[nodiscard]] uint16_t getPacketDataLen() const override;
  [[nodiscard]] uint8_t getPusVersion() const override;
  [[nodiscard]] uint8_t getAcknowledgeFlags() const override;
  [[nodiscard]] uint8_t getService() const override;
  [[nodiscard]] uint8_t getSubService() const override;
  [[nodiscard]] uint16_t getSourceId() const override;
  [[nodiscard]] const uint8_t* getApplicationData() const override;
  [[nodiscard]] uint16_t getApplicationDataSize() const override;
  [[nodiscard]] uint16_t getErrorControl() const override;

 protected:
  /**
   * With this method, the packet data pointer can be redirected to another
   * location.
   * This call overwrites the parent's setData method to set both its
   * @c tc_data pointer and the parent's @c data pointer.
   *
   * @param p_data    A pointer to another PUS Telecommand Packet.
   */
  ReturnValue_t setData(uint8_t* pData, size_t maxSize, void* args) override = 0;
  SpacePacketReader spReader;
  /**
   * This struct defines the data structure of a Space Packet when accessed
   * via a pointer.
   * @ingroup tmtcpackets
   */
  struct PusTcPointers {
    const uint8_t* spHeaderStart;
    const uint8_t* secHeaderStart;
    const uint8_t* userDataStart;
  };

  PusTcPointers pointers{};
  size_t maxSize = 0;
};

#endif /* TMTCPACKET_PUS_TCPACKETBASE_H_ */
