#ifndef FSFW_TMTCPACKET_CFDP_CFDPPACKETSTORED_H_
#define FSFW_TMTCPACKET_CFDP_CFDPPACKETSTORED_H_

#include "CfdpPacket.h"
#include "fsfw/storagemanager/storeAddress.h"

class CfdpPacketStored : public CfdpReader {
 public:
  /**
   * Create stored packet with existing data.
   * @param data
   * @param size
   */
  CfdpPacketStored(const uint8_t* data, size_t size);
  /**
   * Create stored packet from existing packet in store
   * @param setAddress
   */
  explicit CfdpPacketStored(store_address_t setAddress);
  CfdpPacketStored();

  ~CfdpPacketStored() override;

  /**
   * Getter function for the raw data.
   * @param dataPtr [out] Pointer to the data pointer to set
   * @param dataSize [out] Address of size to set.
   * @return -@c returnvalue::OK if data was retrieved successfully.
   */
  ReturnValue_t getData(const uint8_t** dataPtr, size_t* dataSize);

  void setStoreAddress(store_address_t setAddress);

  store_address_t getStoreAddress();

  ReturnValue_t deletePacket();

 private:
  bool isSizeCorrect();

 protected:
  /**
   * The address where the packet data of the object instance is stored.
   */
  store_address_t storeAddress;
  /**
   * A helper method to check if a store is assigned to the class.
   * If not, the method tries to retrieve the store from the global
   * ObjectManager.
   * @return  @li @c true if the store is linked or could be created.
   *          @li @c false otherwise.
   */
  bool checkAndSetStore();
};

#endif /* FSFW_TMTCPACKET_CFDP_CFDPPACKETSTORED_H_ */
