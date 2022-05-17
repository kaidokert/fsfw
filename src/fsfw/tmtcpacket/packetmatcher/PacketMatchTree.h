#ifndef FSFW_TMTCPACKET_PACKETMATCHER_PACKETMATCHTREE_H_
#define FSFW_TMTCPACKET_PACKETMATCHER_PACKETMATCHTREE_H_

#include "fsfw/container/PlacementFactory.h"
#include "fsfw/globalfunctions/matching/MatchTree.h"
#include "fsfw/storagemanager/LocalPool.h"
#include "fsfw/tmtcpacket/pus/tm/TmPacketMinimal.h"

class PacketMatchTree : public MatchTree<TmPacketMinimal*>, public HasReturnvaluesIF {
 public:
  PacketMatchTree(Node* root);
  PacketMatchTree(iterator root);
  PacketMatchTree();
  virtual ~PacketMatchTree();
  ReturnValue_t changeMatch(bool addToMatch, uint16_t apid, uint8_t type = 0, uint8_t subtype = 0);
  ReturnValue_t addMatch(uint16_t apid, uint8_t type = 0, uint8_t subtype = 0);
  ReturnValue_t removeMatch(uint16_t apid, uint8_t type = 0, uint8_t subtype = 0);
  ReturnValue_t initialize();

 protected:
  ReturnValue_t cleanUpElement(iterator position);

 private:
  static const uint8_t N_POOLS = 4;
  LocalPool factoryBackend;
  PlacementFactory factory;
  static const LocalPool::LocalPoolConfig poolConfig;
  static const uint16_t POOL_SIZES[N_POOLS];
  static const uint16_t N_ELEMENTS[N_POOLS];
  template <typename VALUE_T, typename INSERTION_T>
  ReturnValue_t findOrInsertMatch(iterator startAt, VALUE_T test, iterator* lastTest);
  iterator findMatch(iterator startAt, TmPacketMinimal* test);
};

#endif /* FRAMEWORK_TMTCPACKET_PACKETMATCHER_PACKETMATCHTREE_H_ */
