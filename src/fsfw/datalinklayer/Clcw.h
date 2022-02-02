#ifndef CLCW_H_
#define CLCW_H_

#include "ClcwIF.h"
#include "dllConf.h"

/**
 * Small helper method to handle the Clcw values.
 * It has a content struct that manages the register and can be set externally.
 * @ingroup ccsds_handling
 */
class Clcw : public ClcwIF {
 private:
  static const uint8_t STATUS_FIELD_DEFAULT = 0b00000001;  //!< Default for the status field.
  static const uint8_t NO_RF_AVIALABLE_POSITION =
      7;  //!< Position of a flag in the register (starting with 0).
  static const uint8_t NO_BIT_LOCK_POSITION =
      6;  //!< Position of a flag in the register (starting with 0).
  static const uint8_t LOCKOUT_FLAG_POSITION =
      5;  //!< Position of a flag in the register (starting with 0).
  static const uint8_t WAIT_FLAG_POSITION =
      4;  //!< Position of a flag in the register (starting with 0).
  static const uint8_t RETRANSMIT_FLAG_POSITION =
      3;  //!< Position of a flag in the register (starting with 0).
  static const uint8_t NO_RF_AVIALABLE_MASK =
      0xFF xor (1 << NO_RF_AVIALABLE_POSITION);  //!< Mask for a flag in the register.
  static const uint8_t NO_BIT_LOCK_MASK =
      0xFF xor (1 << NO_BIT_LOCK_POSITION);  //!< Mask for a flag in the register.
  static const uint8_t LOCKOUT_FLAG_MASK =
      0xFF xor (1 << LOCKOUT_FLAG_POSITION);  //!< Mask for a flag in the register.
  static const uint8_t WAIT_FLAG_MASK =
      0xFF xor (1 << WAIT_FLAG_POSITION);  //!< Mask for a flag in the register.
  static const uint8_t RETRANSMIT_FLAG_MASK =
      0xFF xor (1 << RETRANSMIT_FLAG_POSITION);         //!< Mask for a flag in the register.
  static const uint8_t FARM_B_COUNT_MASK = 0b11111001;  //!< Mask for a counter in the register.
  /**
   * This is the data structure of the CLCW register.
   */
  union clcwContent {
    uint32_t raw;
    struct {
      uint8_t status;
      uint8_t virtualChannelIdSpare;
      uint8_t flags;
      uint8_t vRValue;
    };
  };
  clcwContent content;  //!< Represents the content of the register.
 public:
  /**
   * The constructor sets everything to default values.
   */
  Clcw();
  /**
   * Nothing happens in the destructor.
   */
  ~Clcw();
  void setVirtualChannel(uint8_t setChannel);
  void setLockoutFlag(bool lockout);
  void setWaitFlag(bool waitFlag);
  void setRetransmitFlag(bool retransmitFlag);
  void setFarmBCount(uint8_t count);
  void setReceiverFrameSequenceNumber(uint8_t vR);
  void setRFAvailable(bool rfAvailable);
  void setBitLock(bool bitLock);
  uint32_t getAsWhole();
  void setWhole(uint32_t rawClcw);
  void print();
};

#endif /* CLCW_H_ */
