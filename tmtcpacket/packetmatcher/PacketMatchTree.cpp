#include "ApidMatcher.h"
#include "PacketMatchTree.h"
#include "ServiceMatcher.h"
#include "SubserviceMatcher.h"

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
	TmPacketMinimal::TmPacketMinimalPointer data;
	data.data_field.service_type = type;
	data.data_field.service_subtype = subtype;
	TmPacketMinimal testPacket((uint8_t*) &data);
	testPacket.setAPID(apid);
	iterator lastTest;
	iterator rollback;
	ReturnValue_t result = findOrInsertMatch<TmPacketMinimal*, ApidMatcher>(
			this->begin(), &testPacket, &lastTest);
	if (result == NEW_NODE_CREATED) {
		rollback = lastTest;
	} else if (result != RETURN_OK) {
		return result;
	}
	if (type == 0) {
		//Check if lastTest has no children, otherwise, delete them,
		//as a more general check is requested.
		if (lastTest.left() != this->end()) {
			removeElementAndAllChildren(lastTest.left());
		}
		return RETURN_OK;
	}
	//Type insertion required.
	result = findOrInsertMatch<TmPacketMinimal*, ServiceMatcher>(
			lastTest.left(), &testPacket, &lastTest);
	if (result == NEW_NODE_CREATED) {
		if  (rollback == this->end()) {
			rollback = lastTest;
		}
	} else if (result != RETURN_OK) {
		if (rollback != this->end()) {
			removeElementAndAllChildren(rollback);
		}
		return result;
	}
	if (subtype == 0) {
		if (lastTest.left() != this->end()) {
			//See above
			removeElementAndAllChildren(lastTest.left());
		}
		return RETURN_OK;
	}
	//Subtype insertion required.
	result = findOrInsertMatch<TmPacketMinimal*, SubServiceMatcher>(
			lastTest.left(), &testPacket, &lastTest);
	if (result == NEW_NODE_CREATED) {
		return RETURN_OK;
	} else if (result != RETURN_OK) {
		if (rollback != this->end()) {
			removeElementAndAllChildren(rollback);
		}
		return result;
	}
	return RETURN_OK;
}

template<typename VALUE_T, typename INSERTION_T>
ReturnValue_t PacketMatchTree::findOrInsertMatch(iterator startAt, VALUE_T test,
		iterator* lastTest) {
	bool attachToBranch = AND;
	iterator iter = startAt;
	while (iter != this->end()) {
		bool isMatch = iter->match(test);
		attachToBranch = OR;
		*lastTest = iter;
		if (isMatch) {
			return RETURN_OK;
		} else {
			//Go down OR branch.
			iter = iter.right();
		}
	}
	//Only reached if nothing was found.
	SerializeableMatcherIF<VALUE_T>* newContent = factory.generate<INSERTION_T>(
			test);
	if (newContent == NULL) {
		return FULL;
	}
	Node* newNode = factory.generate<Node>(newContent);
	if (newNode == NULL) {
		//Need to make sure partially generated content is deleted, otherwise, that's a leak.
		factory.destroy<INSERTION_T>(static_cast<INSERTION_T*>(newContent));
		return FULL;
	}
	*lastTest = insert(attachToBranch, *lastTest, newNode);
	if (*lastTest == end()) {
		//This actaully never fails, so creating a dedicated returncode seems an overshoot.
		return RETURN_FAILED;
	}
	return NEW_NODE_CREATED;
}

ReturnValue_t PacketMatchTree::removeMatch(uint16_t apid, uint8_t type,
		uint8_t subtype) {
	TmPacketMinimal::TmPacketMinimalPointer data;
	data.data_field.service_type = type;
	data.data_field.service_subtype = subtype;
	TmPacketMinimal testPacket((uint8_t*) &data);
	testPacket.setAPID(apid);
	iterator foundElement = findMatch(begin(), &testPacket);
	if (foundElement == this->end()) {
		return NO_MATCH;
	}
	if (type == 0) {
		if (foundElement.left() == end()) {
			return removeElementAndReconnectChildren(foundElement);
		} else {
			return TOO_GENERAL_REQUEST;
		}
	}
	//Go down AND branch. Will abort if empty.
	foundElement = findMatch(foundElement.left(), &testPacket);
	if (foundElement == this->end()) {
		return NO_MATCH;
	}
	if (subtype == 0) {
		if (foundElement.left() == end()) {
			return removeElementAndReconnectChildren(foundElement);
		} else {
			return TOO_GENERAL_REQUEST;
		}
	}
	//Again, go down AND branch.
	foundElement = findMatch(foundElement.left(), &testPacket);
	if (foundElement == end()) {
		return NO_MATCH;
	}
	return removeElementAndReconnectChildren(foundElement);
}

PacketMatchTree::iterator PacketMatchTree::findMatch(iterator startAt,
		TmPacketMinimal* test) {
	iterator iter = startAt;
	while (iter != end()) {
		bool isMatch = iter->match(test);
		if (isMatch) {
			break;
		} else {
			iter = iter.right(); //next OR element
		}
	}
	return iter;
}

ReturnValue_t PacketMatchTree::initialize() {
	return factoryBackend.initialize();
}

const uint16_t PacketMatchTree::POOL_SIZES[N_POOLS] = { sizeof(ServiceMatcher),
		sizeof(SubServiceMatcher), sizeof(ApidMatcher),
		sizeof(PacketMatchTree::Node) };
//Maximum number of types and subtypes to filter should be more than sufficient.
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
	factory.destroy(position.element->value);
	//Go on anyway, there's nothing we can do.
	//SHOULDDO: Throw event, or write debug message?
	return factory.destroy(position.element);
}
