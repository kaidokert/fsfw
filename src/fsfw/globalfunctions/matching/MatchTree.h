#ifndef FRAMEWORK_GLOBALFUNCTIONS_MATCHING_MATCHTREE_H_
#define FRAMEWORK_GLOBALFUNCTIONS_MATCHING_MATCHTREE_H_

#include "../../container/BinaryTree.h"
#include "../../serialize/SerializeAdapter.h"
#include "SerializeableMatcherIF.h"

template <typename T>
class MatchTree : public SerializeableMatcherIF<T>, public BinaryTree<SerializeableMatcherIF<T>> {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::MATCH_TREE_CLASS;
  static const ReturnValue_t TOO_DETAILED_REQUEST = MAKE_RETURN_CODE(1);
  static const ReturnValue_t TOO_GENERAL_REQUEST = MAKE_RETURN_CODE(2);
  static const ReturnValue_t NO_MATCH = MAKE_RETURN_CODE(3);
  static const ReturnValue_t FULL = MAKE_RETURN_CODE(4);
  static const ReturnValue_t NEW_NODE_CREATED = MAKE_RETURN_CODE(5);

  typedef typename BinaryTree<SerializeableMatcherIF<T>>::iterator iterator;
  typedef BinaryNode<SerializeableMatcherIF<T>> Node;
  static const bool AND = true;  // LEFT
  static const bool OR = false;  // RIGHT
  MatchTree(BinaryNode<SerializeableMatcherIF<T>>* root, uint8_t maxDepth = -1)
      : BinaryTree<SerializeableMatcherIF<T>>(root), maxDepth(maxDepth) {}
  MatchTree(iterator root, uint8_t maxDepth = -1)
      : BinaryTree<SerializeableMatcherIF<T>>(root.element), maxDepth(maxDepth) {}
  MatchTree() : BinaryTree<SerializeableMatcherIF<T>>(), maxDepth(-1) {}
  virtual ~MatchTree() {}
  virtual bool match(T number) override { return matchesTree(number); }
  bool matchesTree(T number) {
    iterator iter = this->begin();
    if (iter == this->end()) {
      return false;
    }
    return matchSubtree(iter, number);
  }

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          SerializeIF::Endianness streamEndianness) const override {
    iterator iter = this->begin();
    uint8_t count = this->countRight(iter);
    ReturnValue_t result =
        SerializeAdapter::serialize(&count, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    if (iter == this->end()) {
      return returnvalue::OK;
    }
    result = iter->serialize(buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    if (maxDepth > 0) {
      MatchTree<T> temp(iter.left(), maxDepth - 1);
      result = temp.serialize(buffer, size, maxSize, streamEndianness);
    }
    if (result != returnvalue::OK) {
      return result;
    }
    iter = iter.right();
    while (iter != this->end()) {
      result = iter->serialize(buffer, size, maxSize, streamEndianness);
      if (result != returnvalue::OK) {
        return result;
      }
      if (maxDepth > 0) {
        MatchTree<T> temp(iter.left(), maxDepth - 1);
        result = temp.serialize(buffer, size, maxSize, streamEndianness);
      }
      if (result != returnvalue::OK) {
        return result;
      }
      iter = iter.right();
    }
    return result;
  }

  size_t getSerializedSize() const override {
    // Analogous to serialize!
    uint32_t size = 1;  // One for count
    iterator iter = this->begin();
    if (iter == this->end()) {
      return size;
    }
    // Count object itself
    size += iter->getSerializedSize();
    // Handle everything below on AND side
    if (maxDepth > 0) {
      MatchTree<T> temp(iter.left(), maxDepth - 1);
      size += temp.getSerializedSize();
    }
    // Handle everything on OR side
    iter = iter.right();
    // Iterate over every object on the OR branch
    while (iter != this->end()) {
      size += iter->getSerializedSize();
      if (maxDepth > 0) {
        // If we are allowed to go deeper, handle AND elements.
        MatchTree<T> temp(iter.left(), maxDepth - 1);
        size += temp.getSerializedSize();
      }
      iter = iter.right();
    }
    return size;
  }

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            SerializeIF::Endianness streamEndianness) override {
    return returnvalue::OK;
  }

 protected:
  bool isOnAndBranch(iterator position) {
    if ((position == this->end()) || (position.up() == this->end())) {
      return false;
    }
    if (position.up().left() == position) {
      return true;
    } else {
      return false;
    }
  }

  // SHOULDDO: What to do if insertion/deletion fails. Throw event?
  ReturnValue_t removeElementAndAllChildren(iterator position) {
    auto children = this->erase(position);
    ReturnValue_t result = returnvalue::OK;
    if (children.first != this->end()) {
      result = removeElementAndAllChildren(children.first);
    }
    if (result != returnvalue::OK) {
      return result;
    }
    if (children.second != this->end()) {
      result = removeElementAndAllChildren(children.second);
    }
    if (result != returnvalue::OK) {
      return result;
    }
    // Delete element itself.
    return cleanUpElement(position);
  }

  ReturnValue_t removeElementAndReconnectChildren(iterator position) {
    if (position == this->end()) {
      return returnvalue::OK;
    }
    // Delete everything from the AND branch.
    ReturnValue_t result = returnvalue::OK;
    if (position.left() != this->end()) {
      result = removeElementAndAllChildren(position.left());
      if (result != returnvalue::OK) {
        return result;
      }
    }

    if (position.right() != this->end()) {
      // There's something at the OR branch, reconnect to parent.
      if (isOnAndBranch(position)) {
        // Either one hierarchy up AND branch...
        this->insert(AND, position.up(), position.right().element);
      } else {
        // or on another OR'ed element (or install new root node).
        this->insert(OR, position.up(), position.right().element);
      }
    } else {
      if (isOnAndBranch(position)) {
        // Recursively delete parent node as well, because it is not expected to be there anymore.
        return removeElementAndReconnectChildren(position.up());
      } else {
        // simply delete self.
        this->erase(position);
      }
    }
    // Delete element itself.
    return cleanUpElement(position);
  }

  virtual ReturnValue_t cleanUpElement(iterator position) { return returnvalue::OK; }

  bool matchSubtree(iterator iter, T number) {
    if (iter == nullptr) {
      return false;
    }
    bool isMatch = iter->match(number);
    if (isMatch) {
      if (iter.left() == this->end()) {
        return true;
      }
      isMatch = matchSubtree(iter.left(), number);
      if (isMatch) {
        return true;
      }
    }
    if (iter.right() == this->end()) {
      return false;
    }
    return matchSubtree(iter.right(), number);
  }

 private:
  uint8_t maxDepth;
};

#endif /* FRAMEWORK_GLOBALFUNCTIONS_MATCHING_MATCHTREE_H_ */
