#ifndef DECIMALMATCHER_H_
#define DECIMALMATCHER_H_

#include "MatcherIF.h"

template <typename T>
class DecimalMatcher : public MatcherIF<T> {
 public:
  bool inverted;
  T mask, matchField;

  DecimalMatcher() : inverted(false), mask(0), matchField(0) {}

  DecimalMatcher(T mask, T match, bool inverted = false)
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
    T decimal = 1, remainderMask, remainderMatch, remainderInput;

    while (mask != 0) {
      remainderMask = mask % (decimal * 10);
      remainderMatch = match % (decimal * 10);
      remainderInput = input % (decimal * 10);
      if (remainderMask != 0) {
        if (remainderMatch != remainderInput) {
          return false;
        }
      }
      mask -= remainderMask;
      match -= remainderMatch;
      input -= remainderInput;
      decimal *= 10;
    }
    return true;
  }
};

#endif /* DECIMALMATCHER_H_ */
