#ifndef FRAMEWORK_GLOBALFUNCTIONS_MATCHING_SERIALIZEABLEMATCHERIF_H_
#define FRAMEWORK_GLOBALFUNCTIONS_MATCHING_SERIALIZEABLEMATCHERIF_H_

#include <framework/globalfunctions/matching/MatcherIF.h>
#include <framework/serialize/SerializeIF.h>

template<typename T>
class SerializeableMatcherIF : public MatcherIF<T>, public SerializeIF {
public:
	virtual ~SerializeableMatcherIF() {}
};

#endif /* FRAMEWORK_GLOBALFUNCTIONS_MATCHING_SERIALIZEABLEMATCHERIF_H_ */
