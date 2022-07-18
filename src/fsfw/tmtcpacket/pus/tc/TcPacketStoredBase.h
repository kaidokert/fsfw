#ifndef TMTCPACKET_PUS_TCPACKETSTORED_H_
#define TMTCPACKET_PUS_TCPACKETSTORED_H_

#include "PusTcReader.h"
#include "TcPacketStoredIF.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

/**
 *	Base class for telecommand packets like CFDP or PUS packets.
 *	@ingroup tmtcpackets
 */
class TcPacketStoredBase : public TcPacketStoredIF {
 public:
  /**
   * This is a default constructor which does not set the data pointer to initialize
   * with an empty cached store address
   */
  TcPacketStoredBase();
  /**
   * Constructor to set to an existing store address.
   * @param setAddress
   */
  explicit TcPacketStoredBase(store_address_t setAddress);
  /**
   * Another constructor to create a TcPacket from a raw packet stream.
   * Takes the data and adds it unchecked to the TcStore.
   * @param data  Pointer to the complete TC Space Packet.
   * @param Size size of the packet.
   */
  TcPacketStoredBase(const uint8_t* data, uint32_t size);

  virtual ~TcPacketStoredBase();

  /**
   * Getter function for the raw data.
   * @param dataPtr [out] Pointer to the data pointer to set
   * @param dataSize [out] Address of size to set.
   * @return -@c RETURN_OK if data was retrieved successfully.
   */
  ReturnValue_t getData(const uint8_t** dataPtr, size_t* dataSize) override;

  void setStoreAddress(store_address_t setAddress, RedirectableDataPointerIF* packet) override;
  store_address_t getStoreAddress() override;

  /**
   * With this call, the packet is deleted.
   * It removes itself from the store and sets its data pointer to NULL.
   * @return	returncode from deleting the data.
   */
  virtual ReturnValue_t deletePacket() = 0;

  /**
   * This method performs a size check.
   * It reads the stored size and compares it with the size entered in the
   * packet header. This class is the optimal place for such a check as it
   * has access to both the header data and the store.
   * @return	true if size is correct, false if packet is not registered in
   * 			store or size is incorrect.
   */
  virtual bool isSizeCorrect() = 0;

 protected:
  /**
   * This is a pointer to the store all instances of the class use.
   * If the store is not yet set (i.e. @c store is NULL), every constructor
   * call tries to set it and throws an error message in case of failures.
   * The default store is objects::TC_STORE.
   */
  static StorageManagerIF* STORE;
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
  static bool checkAndSetStore();
};

#endif /* TMTCPACKET_PUS_TCPACKETSTORED_H_ */
