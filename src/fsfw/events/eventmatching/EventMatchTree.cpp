#include "fsfw/events/eventmatching/EventMatchTree.h"

#include "fsfw/events/eventmatching/EventIdRangeMatcher.h"
#include "fsfw/events/eventmatching/ReporterRangeMatcher.h"
#include "fsfw/events/eventmatching/SeverityRangeMatcher.h"

EventMatchTree::EventMatchTree(StorageManagerIF* storageBackend, bool invertedMatch)
    : MatchTree<EventMessage*>(end(), 1), factory(storageBackend), invertedMatch(invertedMatch) {}

EventMatchTree::~EventMatchTree() {}

bool EventMatchTree::match(EventMessage* number) {
  if (invertedMatch) {
    return !MatchTree<EventMessage*>::match(number);
  } else {
    return MatchTree<EventMessage*>::match(number);
  }
}

ReturnValue_t EventMatchTree::addMatch(EventId_t idFrom, EventId_t idTo, bool idInverted,
                                       object_id_t reporterFrom, object_id_t reporterTo,
                                       bool reporterInverted) {
  if (idFrom == 0) {
    // Assuming all events shall be forwarded.
    idTo = 0;
    idInverted = true;
  }
  if (idTo == 0) {
    idTo = idFrom;
  }
  iterator lastTest;
  ReturnValue_t result = findOrInsertRangeMatcher<EventId_t, EventIdRangeMatcher>(
      begin(), idFrom, idTo, idInverted, &lastTest);
  if (result != returnvalue::OK) {
    return result;
  }
  if (reporterFrom == 0) {
    // No need to add another AND branch
    return returnvalue::OK;
  }
  if (reporterTo == 0) {
    reporterTo = reporterFrom;
  }
  return findOrInsertRangeMatcher<object_id_t, ReporterRangeMatcher>(
      lastTest.left(), reporterFrom, reporterTo, reporterInverted, &lastTest);
}

ReturnValue_t EventMatchTree::removeMatch(EventId_t idFrom, EventId_t idTo, bool idInverted,
                                          object_id_t reporterFrom, object_id_t reporterTo,
                                          bool reporterInverted) {
  iterator foundElement;

  if (idFrom == 0) {
    // Assuming a "forward all events" request.
    idTo = 0;
    idInverted = true;
  }
  if (idTo == 0) {
    idTo = idFrom;
  }
  foundElement =
      findRangeMatcher<EventId_t, EventIdRangeMatcher>(begin(), idFrom, idTo, idInverted);
  if (foundElement == end()) {
    return NO_MATCH;  // Can't tell if too detailed or just not found.
  }
  if (reporterFrom == 0) {
    if (foundElement.left() == end()) {
      return removeElementAndReconnectChildren(foundElement);
    } else {
      return TOO_GENERAL_REQUEST;
    }
  }
  if (reporterTo == 0) {
    reporterTo = reporterFrom;
  }
  foundElement = findRangeMatcher<object_id_t, ReporterRangeMatcher>(
      foundElement.left(), reporterFrom, reporterTo, reporterInverted);
  if (foundElement == end()) {
    return NO_MATCH;
  } else {
    return removeElementAndReconnectChildren(foundElement);
  }
}

template <typename VALUE_T, typename INSERTION_T>
inline ReturnValue_t EventMatchTree::findOrInsertRangeMatcher(iterator start, VALUE_T idFrom,
                                                              VALUE_T idTo, bool inverted,
                                                              iterator* lastTest) {
  bool attachToBranch = AND;
  iterator iter = start;
  while (iter != end()) {
    INSERTION_T* matcher = static_cast<INSERTION_T*>(*iter);
    attachToBranch = OR;
    *lastTest = iter;
    if ((matcher->rangeMatcher.lowerBound == idFrom) &&
        (matcher->rangeMatcher.upperBound == idTo) &&
        (matcher->rangeMatcher.inverted == inverted)) {
      return returnvalue::OK;
    } else {
      iter = iter.right();
    }
  }
  // Only reached if nothing was found.
  SerializeableMatcherIF<EventMessage*>* newContent =
      factory.generate<INSERTION_T>(idFrom, idTo, inverted);
  if (newContent == NULL) {
    return FULL;
  }
  Node* newNode = factory.generate<Node>(newContent);
  if (newNode == NULL) {
    // Need to make sure partially generated content is deleted, otherwise, that's a leak.
    factory.destroy<INSERTION_T>(static_cast<INSERTION_T*>(newContent));
    return FULL;
  }
  *lastTest = insert(attachToBranch, *lastTest, newNode);
  if (*lastTest == end()) {
    // This actaully never fails, so creating a dedicated returncode seems an overshoot.
    return returnvalue::FAILED;
  }
  return returnvalue::OK;
}

template <typename VALUE_T, typename INSERTION_T>
EventMatchTree::iterator EventMatchTree::findRangeMatcher(iterator start, VALUE_T idFrom,
                                                          VALUE_T idTo, bool inverted) {
  iterator iter = start;
  while (iter != end()) {
    INSERTION_T* matcher = static_cast<INSERTION_T*>(*iter);
    if ((matcher->rangeMatcher.lowerBound == idFrom) &&
        (matcher->rangeMatcher.upperBound == idTo) &&
        (matcher->rangeMatcher.inverted == inverted)) {
      break;
    } else {
      iter = iter.right();  // next OR element
    }
  }
  return iter;
}

ReturnValue_t EventMatchTree::cleanUpElement(iterator position) {
  factory.destroy(position.element->value);
  // If deletion fails, delete element anyway, nothing we can do.
  // SHOULDO: Throw event, or write debug output.
  return factory.destroy(position.element);
}
