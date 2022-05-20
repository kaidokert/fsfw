#ifndef FSFW_TMTCPACKET_PUS_TCPACKETSTOREDIF_H_
#define FSFW_TMTCPACKET_PUS_TCPACKETSTOREDIF_H_

#include <fsfw/tmtcpacket/RedirectableDataPointerIF.h>

#include "TcPacketPusBase.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include "fsfw/storagemanager/storeAddress.h"

class TcPacketStoredIF {
 public:
  virtual ~TcPacketStoredIF()= default;;

  /**
   * With this call, the stored packet can be set to another packet in a store. This is useful
   * if the packet is a class member and used for more than one packet.
   * @param setAddress    The new packet id to link to.
   */
  virtual void setStoreAddress(store_address_t setAddress, RedirectableDataPointerIF* packet) = 0;

  virtual store_address_t getStoreAddress() = 0;

  /**
   * Getter function for the raw data.
   * @param dataPtr [out]     Pointer to the data pointer to set
   * @param dataSize [out]    Address of size to set.
   * @return -@c RETURN_OK if data was retrieved successfully.
   */
  virtual ReturnValue_t getData(const uint8_t** dataPtr, size_t* dataSize) = 0;
};

#endif /* FSFW_TMTCPACKET_PUS_TCPACKETSTOREDIF_H_ */
