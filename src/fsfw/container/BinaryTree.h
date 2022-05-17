#ifndef FRAMEWORK_CONTAINER_BINARYTREE_H_
#define FRAMEWORK_CONTAINER_BINARYTREE_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
template <typename Tp>
class BinaryNode {
 public:
  BinaryNode(Tp* setValue) : value(setValue), left(NULL), right(NULL), parent(NULL) {}
  Tp* value;
  BinaryNode* left;
  BinaryNode* right;
  BinaryNode* parent;
};

template <typename Tp>
class ExplicitNodeIterator {
 public:
  typedef ExplicitNodeIterator<Tp> _Self;
  typedef BinaryNode<Tp> _Node;
  typedef Tp value_type;
  typedef Tp* pointer;
  typedef Tp& reference;
  ExplicitNodeIterator() : element(NULL) {}
  ExplicitNodeIterator(_Node* node) : element(node) {}
  BinaryNode<Tp>* element;
  _Self up() { return _Self(element->parent); }
  _Self left() {
    if (element != NULL) {
      return _Self(element->left);
    } else {
      return _Self(NULL);
    }
  }
  _Self right() {
    if (element != NULL) {
      return _Self(element->right);
    } else {
      return _Self(NULL);
    }
  }
  bool operator==(const _Self& __x) const { return element == __x.element; }
  bool operator!=(const _Self& __x) const { return element != __x.element; }
  pointer operator->() const {
    if (element != NULL) {
      return element->value;
    } else {
      return NULL;
    }
  }
  pointer operator*() const { return this->operator->(); }
};

/**
 * Pretty rudimentary version of a simple binary tree (not a binary search tree!).
 */
template <typename Tp>
class BinaryTree {
 public:
  typedef ExplicitNodeIterator<Tp> iterator;
  typedef BinaryNode<Tp> Node;
  typedef std::pair<iterator, iterator> children;
  BinaryTree() : rootNode(NULL) {}
  BinaryTree(Node* rootNode) : rootNode(rootNode) {}
  iterator begin() const { return iterator(rootNode); }
  static iterator end() { return iterator(NULL); }
  iterator insert(bool insertLeft, iterator parentNode, Node* newNode) {
    newNode->parent = parentNode.element;
    if (parentNode.element != NULL) {
      if (insertLeft) {
        parentNode.element->left = newNode;
      } else {
        parentNode.element->right = newNode;
      }
    } else {
      // Insert first element.
      rootNode = newNode;
    }
    return iterator(newNode);
  }
  // No recursion to children. Needs to be done externally.
  children erase(iterator node) {
    if (node.element == rootNode) {
      // We're root node
      rootNode = NULL;
    } else {
      // Delete parent's reference
      if (node.up().left() == node) {
        node.up().element->left = NULL;
      } else {
        node.up().element->right = NULL;
      }
    }
    return children(node.element->left, node.element->right);
  }
  static uint32_t countLeft(iterator start) {
    if (start == end()) {
      return 0;
    }
    // We also count the start node itself.
    uint32_t count = 1;
    while (start.left() != end()) {
      count++;
      start = start.left();
    }
    return count;
  }
  static uint32_t countRight(iterator start) {
    if (start == end()) {
      return 0;
    }
    // We also count the start node itself.
    uint32_t count = 1;
    while (start.right() != end()) {
      count++;
      start = start.right();
    }
    return count;
  }

 protected:
  Node* rootNode;
};

#endif /* FRAMEWORK_CONTAINER_BINARYTREE_H_ */
