#ifndef MATCHERIF_H_
#define MATCHERIF_H_

template <typename T>
class MatcherIF {
 public:
  virtual ~MatcherIF() {}

  virtual bool match(T number) { return false; }
};

#endif /* MATCHERIF_H_ */
