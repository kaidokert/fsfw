#ifndef TMTCPACKET_PUS_TMPACKETBASE_H_
#define TMTCPACKET_PUS_TMPACKETBASE_H_

#include "fsfw/objectmanager/SystemObjectIF.h"
#include "fsfw/timemanager/Clock.h"
#include "fsfw/timemanager/TimeStamperIF.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketReader.h"

namespace Factory {

void setStaticFrameworkObjectIds();

}

/**
 * This class is the basic data handler for any ECSS PUS Telemetry packet.
 *
 * In addition to #SpacePacketBase, the class provides methods to handle
 * the standardized entries of the PUS TM Packet Data Field Header.
 * It does not contain the packet data itself but a pointer to the
 * data must be set on instantiation. An invalid pointer may cause
 * damage, as no getter method checks data validity. Anyway, a NULL
 * check can be performed by making use of the getWholeData method.
 * @ingroup tmtcpackets
 */
class TmPacketBase : public SpacePacketReader {
  friend void(Factory::setStaticFrameworkObjectIds)();

 public:
  //! Maximum size of a TM Packet in this mission.
  //! TODO: Make this dependant on a config variable.
  static const uint32_t MISSION_TM_PACKET_MAX_SIZE = 2048;

  /**
   * This is the default constructor.
   * It sets its internal data pointer to the address passed and also
   * forwards the data pointer to the parent SpacePacketBase class.
   * @param set_address	The position where the packet data lies.
   */
  TmPacketBase(uint8_t* setData);
  /**
   * This is the empty default destructor.
   */
  virtual ~TmPacketBase();

  /**
   * This is a getter for the packet's PUS Service ID, which is the second
   * byte of the Data Field Header.
   * @return	The packet's PUS Service ID.
   */
  virtual uint8_t getService() = 0;
  /**
   * This is a getter for the packet's PUS Service Subtype, which is the
   * third byte of the Data Field Header.
   * @return	The packet's PUS Service Subtype.
   */
  virtual uint8_t getSubService() = 0;
  /**
   * This is a getter for a pointer to the packet's Source data.
   *
   * These are the bytes that follow after the Data Field Header. They form
   * the packet's source data.
   * @return	A pointer to the PUS Source Data.
   */
  virtual uint8_t* getSourceData() = 0;
  /**
   * This method calculates the size of the PUS Source data field.
   *
   * It takes the information stored in the CCSDS Packet Data Length field
   * and subtracts the Data Field Header size and the CRC size.
   * @return	The size of the PUS Source Data (without Error Control field)
   */
  virtual uint16_t getSourceDataSize() = 0;

  /**
   * Get size of data field which can differ based on implementation
   * @return
   */
  virtual uint16_t getDataFieldSize() = 0;

  virtual size_t getPacketMinimumSize() const = 0;

  /**
   * Interprets the "time"-field in the secondary header and returns it in
   * timeval format.
   * @return Converted timestamp of packet.
   */
  virtual ReturnValue_t getPacketTime(timeval* timestamp) const;
  /**
   * Returns a raw pointer to the beginning of the time field.
   * @return Raw pointer to time field.
   */
  virtual uint8_t* getPacketTimeRaw() const = 0;

  virtual size_t getTimestampSize() const = 0;

  /**
   * This is a debugging helper method that prints the whole packet content
   * to the screen.
   */
  void print();
  /**
   * With this method, the Error Control Field is updated to match the
   * current content of the packet. This method is not protected because
   * a recalculation by the user might be necessary when manipulating fields
   * like the sequence count.
   */
  void setErrorControl();
  /**
   * This getter returns the Error Control Field of the packet.
   *
   * The field is placed after any possible Source Data. If no
   * Source Data is present there's still an Error Control field. It is
   * supposed to be a 16bit-CRC.
   * @return  The PUS Error Control
   */
  uint16_t getErrorControl();

 protected:
  /**
   * The timeStamper is responsible for adding a timestamp to the packet.
   * It is initialized lazy.
   */
  static TimeStamperIF* timeStamper;
  //! The ID to use when looking for a time stamper.
  static object_id_t timeStamperId;

  /**
   * Checks if a time stamper is available and tries to set it if not.
   * @return Returns false if setting failed.
   */
  bool checkAndSetStamper();
};

#endif /* TMTCPACKET_PUS_TMPACKETBASE_H_ */
