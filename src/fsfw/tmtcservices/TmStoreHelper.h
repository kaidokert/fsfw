#ifndef FSFW_TMTCSERVICES_STOREHELPER_H
#define FSFW_TMTCSERVICES_STOREHELPER_H

#include "fsfw/internalerror/InternalErrorReporterIF.h"
#include "fsfw/ipc/MessageQueueMessageIF.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/timemanager/TimeStamperIF.h"
#include "fsfw/tmtcpacket/pus/tm/PusTmCreator.h"

class TmStoreHelper {
 public:
  explicit TmStoreHelper(uint16_t defaultApid);
  TmStoreHelper(uint16_t defaultApid, StorageManagerIF& tmStore);
  TmStoreHelper(uint16_t defaultApid, StorageManagerIF& tmStore, TimeStamperIF& timeStamper);

  ReturnValue_t preparePacket(uint8_t service, uint8_t subservice, uint16_t counter);

  void setTimeStamper(TimeStamperIF& timeStamper);
  [[nodiscard]] const store_address_t& getCurrentAddr() const;
  void setSourceDataRaw(const uint8_t* data, size_t len);
  void setSourceDataSerializable(SerializeIF* serializable);
  void setApid(uint16_t apid);
  StorageManagerIF* getTmStore();
  void setTmStore(StorageManagerIF* store);
  ReturnValue_t addPacketToStore();
  ReturnValue_t deletePacket();

 private:
  PusTmCreator creator;
  store_address_t currentAddr{};
  StorageManagerIF* tmStore;
};
#endif  // FSFW_TMTCSERVICES_STOREHELPER_H
