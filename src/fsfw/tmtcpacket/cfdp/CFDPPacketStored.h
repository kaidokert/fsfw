#ifndef FSFW_INC_FSFW_TMTCPACKET_CFDP_CFDPPACKETSTORED_H_
#define FSFW_INC_FSFW_TMTCPACKET_CFDP_CFDPPACKETSTORED_H_

#include "../pus/tc/TcPacketStoredBase.h"
#include "CFDPPacket.h"

class CFDPPacketStored : public CFDPPacket, public TcPacketStoredBase {
 public:
  /**
   * Create stored packet with existing data.
   * @param data
   * @param size
   */
  CFDPPacketStored(const uint8_t* data, size_t size);
  /**
   * Create stored packet from existing packet in store
   * @param setAddress
   */
  explicit CFDPPacketStored(store_address_t setAddress);
  CFDPPacketStored();

  ~CFDPPacketStored() override;

  /**
   * Getter function for the raw data.
   * @param dataPtr [out] Pointer to the data pointer to set
   * @param dataSize [out] Address of size to set.
   * @return -@c RETURN_OK if data was retrieved successfully.
   */
  ReturnValue_t getData(const uint8_t** dataPtr, size_t* dataSize) override;

  void setStoreAddress(store_address_t setAddress, RedirectableDataPointerIF* packet) override;

  store_address_t getStoreAddress() override;

  ReturnValue_t deletePacket() override;

 private:
  bool isSizeCorrect() override;

 protected:
  /**
   * The address where the packet data of the object instance is stored.
   */
  store_address_t storeAddress;
};

#endif /* FSFW_INC_FSFW_TMTCPACKET_CFDP_CFDPPACKETSTORED_H_ */
