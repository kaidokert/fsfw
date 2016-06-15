/*
 * PacketMatchTree.cpp
 *
 *  Created on: 10.03.2015
 *      Author: baetz
 */

#include <framework/tmtcpacket/packetmatcher/ApidMatcher.h>
#include <framework/tmtcpacket/packetmatcher/PacketMatchTree.h>
#include <framework/tmtcpacket/packetmatcher/ServiceMatcher.h>
#include <framework/tmtcpacket/packetmatcher/SubserviceMatcher.h>

PacketMatchTree::PacketMatchTree(Node* root) :
		MatchTree<TmPacketMinimal*>(root, 2), factoryBackend(0, POOL_SIZES,
				N_ELEMENTS, false, true), factory(&factoryBackend) {
}

PacketMatchTree::PacketMatchTree(iterator root) :
		MatchTree<TmPacketMinimal*>(root.element, 2), factoryBackend(0,
				POOL_SIZES, N_ELEMENTS, false, true), factory(&factoryBackend) {
}

PacketMatchTree::PacketMatchTree() :
		MatchTree<TmPacketMinimal*>((Node*) NULL, 2), factoryBackend(0,
				POOL_SIZES, N_ELEMENTS, false, true), factory(&factoryBackend) {
}

PacketMatchTree::~PacketMatchTree() {
}

ReturnValue_t PacketMatchTree::addMatch(uint16_t apid, uint8_t type,
		uint8_t subtype) {
	//We assume adding APID is always requested.
	uint8_t expectedHierarchy = 0;
	if (type != 0) {
		expectedHierarchy++;
		if (subtype != 0) {
			expectedHierarchy++;
		}
	}
	TmPacketMinimal::TmPacketMinimalPointer data;
	data.data_field.service_type = type;
	data.data_field.service_subtype = subtype;
	TmPacketMinimal testPacket((uint8_t*)&data);
	testPacket.setAPID(apid);
	uint8_t realHierarchy = 0;
	iterator lastMatch;
	bool isInTree = matchesTree(&testPacket, &lastMatch, &realHierarchy);
	if (isInTree) {
		//Matches somehow.
		//TODO: Are we interested in details?
		return RETURN_OK;
	}
	if (expectedHierarchy == realHierarchy) {
		//Add another element (of correct type) at the OR branch.
		lastMatch = addElement(OR, realHierarchy, lastMatch, &testPacket);
		if (lastMatch == this->end()) {
			return FULL;
		}
	} else if (expectedHierarchy > realHierarchy) {
		//A certain amount of downward structure does not exist. Add first element as OR, rest as AND.

		lastMatch = addElement(OR, realHierarchy, lastMatch, &testPacket);
		if (lastMatch == end()) {
			return FULL;
		}
		iterator firstOfList = lastMatch;
		while (lastMatch != end() && realHierarchy < expectedHierarchy) {
			realHierarchy++;
			lastMatch = addElement(AND, realHierarchy, lastMatch, &testPacket);
		}
		if (lastMatch == end()) {
			//At least one element could not be inserted. So delete everything.
			removeElementAndAllChildren(firstOfList);
			return FULL;
		}
	} else {
		//Might work like that.
		//Too detailed match, delete the last element and all its children.
		while (lastMatch.up() != end() && lastMatch.up().left() != lastMatch) {
			lastMatch = lastMatch.up();
		}
		removeElementAndAllChildren(lastMatch);
	}
	return RETURN_OK;
}

ReturnValue_t PacketMatchTree::removeMatch(uint16_t apid, uint8_t type,
		uint8_t subtype) {
	//We assume APID is always in request.
	uint8_t expectedHierarchy = 1;
	if (type != 0) {
		expectedHierarchy++;
		if (subtype != 0) {
			expectedHierarchy++;
		}
	}
	TmPacketMinimal::TmPacketMinimalPointer data;
	data.data_field.service_type = type;
	data.data_field.service_subtype = subtype;
	TmPacketMinimal testPacket((uint8_t*)&data);
	testPacket.setAPID(apid);
	uint8_t realHierarchy = 0;
	iterator lastMatch;
	bool isInTree = matchesTree(&testPacket, &lastMatch, &realHierarchy);
	if (isInTree) {
		if (expectedHierarchy == realHierarchy) {
			return removeElementAndReconnectChildren(lastMatch);
		} else {
			return TOO_DETAILED_REQUEST;
		}
	} else {
		//TODO: Maybe refine this a bit.
		return NO_MATCH;
	}
}

PacketMatchTree::iterator PacketMatchTree::addElement(bool andBranch,
		uint8_t level, PacketMatchTree::iterator position,
		TmPacketMinimal* content) {
	SerializeableMatcherIF<TmPacketMinimal*>* newContent = NULL;
	switch (level) {
	case 0:
		newContent = factory.generate<ApidMatcher>(content->getAPID());
		break;
	case 1:
		newContent = factory.generate<ServiceMatcher>(content->getService());
		break;
	case 2:
		newContent = factory.generate<SubServiceMatcher>(
				content->getSubService());
		break;
	default:
		break;
	}
	if (newContent == NULL) {
		return end();
	}
	Node* newNode = factory.generate<Node>(newContent);
	if (newNode == NULL) {
		return end();
	}
	return insert(andBranch, position, newNode);
}

ReturnValue_t PacketMatchTree::initialize() {
	return factoryBackend.initialize();
}

const uint16_t PacketMatchTree::POOL_SIZES[N_POOLS] = { sizeof(ServiceMatcher),
		sizeof(SubServiceMatcher), sizeof(ApidMatcher),
		sizeof(PacketMatchTree::Node) };
//TODO: Rather arbitrary. Adjust!
const uint16_t PacketMatchTree::N_ELEMENTS[N_POOLS] = { 10, 20, 2, 40 };

ReturnValue_t PacketMatchTree::changeMatch(bool addToMatch, uint16_t apid,
		uint8_t type, uint8_t subtype) {
	if (addToMatch) {
		return addMatch(apid, type, subtype);
	} else {
		return removeMatch(apid, type, subtype);
	}
}

ReturnValue_t PacketMatchTree::cleanUpElement(iterator position) {
	//TODO: What if first deletion fails?
	factory.destroy(position.element->value);
	return factory.destroy(position.element);
}
