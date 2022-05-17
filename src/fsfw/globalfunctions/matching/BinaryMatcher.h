#ifndef BINARYMATCHER_H_
#define BINARYMATCHER_H_

#include "MatcherIF.h"

template <typename T>
class BinaryMatcher : public MatcherIF<T> {
 public:
  bool inverted;
  T mask, matchField;

  BinaryMatcher() : inverted(false), mask(0), matchField(0) {}

  BinaryMatcher(T mask, T match, bool inverted = false)
      : inverted(inverted), mask(mask), matchField(match) {}

  bool match(T input) {
    if (inverted) {
      return ~doMatch(input, mask, matchField);
    } else {
      return doMatch(input, mask, matchField);
    }
  }

 protected:
  bool doMatch(T input, T mask, T match) {
    match = match & mask;
    input = input & mask;
    if (input == match) {
      return true;
    } else {
      return false;
    }
  }
};

#endif /* BINARYMATCHER_H_ */
