/*
 * PacketMatchTree.h
 *
 *  Created on: 10.03.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_TMTCPACKET_PACKETMATCHER_PACKETMATCHTREE_H_
#define FRAMEWORK_TMTCPACKET_PACKETMATCHER_PACKETMATCHTREE_H_

#include <framework/container/PlacementFactory.h>
#include <framework/globalfunctions/matching/MatchTree.h>
#include <framework/storagemanager/LocalPool.h>
#include <framework/tmtcpacket/pus/TmPacketMinimal.h>

class PacketMatchTree: public MatchTree<TmPacketMinimal*>, public HasReturnvaluesIF {
public:
	PacketMatchTree(Node* root);
	PacketMatchTree(iterator root);
	PacketMatchTree();
	virtual ~PacketMatchTree();
	ReturnValue_t changeMatch(bool addToMatch, uint16_t apid, uint8_t type = 0,
			uint8_t subtype = 0);
	ReturnValue_t addMatch(uint16_t apid, uint8_t type = 0,
			uint8_t subtype = 0);
	ReturnValue_t removeMatch(uint16_t apid, uint8_t type = 0,
			uint8_t subtype = 0);
	ReturnValue_t initialize();
protected:
	ReturnValue_t cleanUpElement(iterator position);
private:
	static const uint8_t N_POOLS = 4;
	LocalPool<N_POOLS> factoryBackend;
	PlacementFactory factory;
	static const uint16_t POOL_SIZES[N_POOLS];
	static const uint16_t N_ELEMENTS[N_POOLS];
	iterator addElement(bool andBranch, uint8_t level, iterator position, TmPacketMinimal* content);
};

#endif /* FRAMEWORK_TMTCPACKET_PACKETMATCHER_PACKETMATCHTREE_H_ */

