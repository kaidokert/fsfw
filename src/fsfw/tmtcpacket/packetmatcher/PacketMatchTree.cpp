#include "fsfw/tmtcpacket/packetmatcher/PacketMatchTree.h"

#include "fsfw/storagemanager/LocalPool.h"
#include "fsfw/tmtcpacket/packetmatcher/ApidMatcher.h"
#include "fsfw/tmtcpacket/packetmatcher/ServiceMatcher.h"
#include "fsfw/tmtcpacket/packetmatcher/SubserviceMatcher.h"

// This should be configurable..
const LocalPool::LocalPoolConfig PacketMatchTree::poolConfig = {
    {10, sizeof(ServiceMatcher)},
    {20, sizeof(SubServiceMatcher)},
    {2, sizeof(ApidMatcher)},
    {40, sizeof(PacketMatchTree::Node)}};

PacketMatchTree::PacketMatchTree(Node* root)
    : MatchTree<PusTmIF*>(root, 2),
      factoryBackend(0, poolConfig, false, true),
      factory(&factoryBackend) {}

PacketMatchTree::PacketMatchTree(iterator root)
    : MatchTree<PusTmIF*>(root.element, 2),
      factoryBackend(0, poolConfig, false, true),
      factory(&factoryBackend) {}

PacketMatchTree::PacketMatchTree()
    : MatchTree<PusTmIF*>(nullptr, 2),
      factoryBackend(0, poolConfig, false, true),
      factory(&factoryBackend) {}

PacketMatchTree::~PacketMatchTree() = default;

ReturnValue_t PacketMatchTree::addMatch(uint16_t apid, uint8_t type, uint8_t subtype) {
  // We assume adding APID is always requested.
  mintm::MinimalPusTm data{};
  data.secHeader.service = type;
  data.secHeader.subservice = subtype;
  PusTmMinimal testPacket((uint8_t*)&data);
  testPacket.setApid(apid);

  iterator lastTest;
  iterator rollback;
  ReturnValue_t result =
      findOrInsertMatch<PusTmIF*, ApidMatcher>(this->begin(), &testPacket, &lastTest);
  if (result == NEW_NODE_CREATED) {
    rollback = lastTest;
  } else if (result != returnvalue::OK) {
    return result;
  }
  if (type == 0) {
    // Check if lastTest has no children, otherwise, delete them,
    // as a more general check is requested.
    if (lastTest.left() != PacketMatchTree::end()) {
      removeElementAndAllChildren(lastTest.left());
    }
    return returnvalue::OK;
  }
  // Type insertion required.
  result = findOrInsertMatch<PusTmIF*, ServiceMatcher>(lastTest.left(), &testPacket, &lastTest);
  if (result == NEW_NODE_CREATED) {
    if (rollback == PacketMatchTree::end()) {
      rollback = lastTest;
    }
  } else if (result != returnvalue::OK) {
    if (rollback != PacketMatchTree::end()) {
      removeElementAndAllChildren(rollback);
    }
    return result;
  }
  if (subtype == 0) {
    if (lastTest.left() != PacketMatchTree::end()) {
      // See above
      removeElementAndAllChildren(lastTest.left());
    }
    return returnvalue::OK;
  }
  // Subtype insertion required.
  result = findOrInsertMatch<PusTmIF*, SubServiceMatcher>(lastTest.left(), &testPacket, &lastTest);
  if (result == NEW_NODE_CREATED) {
    return returnvalue::OK;
  } else if (result != returnvalue::OK) {
    if (rollback != PacketMatchTree::end()) {
      removeElementAndAllChildren(rollback);
    }
    return result;
  }
  return returnvalue::OK;
}

template <typename VALUE_T, typename INSERTION_T>
ReturnValue_t PacketMatchTree::findOrInsertMatch(iterator startAt, VALUE_T test,
                                                 iterator* lastTest) {
  bool attachToBranch = AND;
  iterator iter = startAt;
  while (iter != PacketMatchTree::end()) {
    bool isMatch = iter->match(test);
    attachToBranch = OR;
    *lastTest = iter;
    if (isMatch) {
      return returnvalue::OK;
    } else {
      // Go down OR branch.
      iter = iter.right();
    }
  }
  // Only reached if nothing was found.
  SerializeableMatcherIF<VALUE_T>* newContent = factory.generate<INSERTION_T>(test);
  if (newContent == nullptr) {
    return FULL;
  }
  Node* newNode = factory.generate<Node>(newContent);
  if (newNode == nullptr) {
    // Need to make sure partially generated content is deleted, otherwise, that's a leak.
    factory.destroy<INSERTION_T>(static_cast<INSERTION_T*>(newContent));
    return FULL;
  }
  *lastTest = insert(attachToBranch, *lastTest, newNode);
  if (*lastTest == end()) {
    // This actaully never fails, so creating a dedicated returncode seems an overshoot.
    return returnvalue::FAILED;
  }
  return NEW_NODE_CREATED;
}

ReturnValue_t PacketMatchTree::removeMatch(uint16_t apid, uint8_t type, uint8_t subtype) {
  mintm::MinimalPusTm data{};
  data.secHeader.service = type;
  data.secHeader.subservice = subtype;
  PusTmMinimal testPacket((uint8_t*)&data);
  testPacket.setApid(apid);
  iterator foundElement = findMatch(begin(), &testPacket);
  if (foundElement == PacketMatchTree::end()) {
    return NO_MATCH;
  }
  if (type == 0) {
    if (foundElement.left() == end()) {
      return removeElementAndReconnectChildren(foundElement);
    } else {
      return TOO_GENERAL_REQUEST;
    }
  }
  // Go down AND branch. Will abort if empty.
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
  // Again, go down AND branch.
  foundElement = findMatch(foundElement.left(), &testPacket);
  if (foundElement == end()) {
    return NO_MATCH;
  }
  return removeElementAndReconnectChildren(foundElement);
}

PacketMatchTree::iterator PacketMatchTree::findMatch(iterator startAt, PusTmIF* test) {
  iterator iter = startAt;
  while (iter != end()) {
    bool isMatch = iter->match(test);
    if (isMatch) {
      break;
    } else {
      iter = iter.right();  // next OR element
    }
  }
  return iter;
}

ReturnValue_t PacketMatchTree::initialize() { return factoryBackend.initialize(); }

ReturnValue_t PacketMatchTree::changeMatch(bool addToMatch, uint16_t apid, uint8_t type,
                                           uint8_t subtype) {
  if (addToMatch) {
    return addMatch(apid, type, subtype);
  } else {
    return removeMatch(apid, type, subtype);
  }
}

ReturnValue_t PacketMatchTree::cleanUpElement(iterator position) {
  factory.destroy(position.element->value);
  // Go on anyway, there's nothing we can do.
  // SHOULDDO: Throw event, or write debug message?
  return factory.destroy(position.element);
}
