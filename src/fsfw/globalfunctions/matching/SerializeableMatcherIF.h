#ifndef FRAMEWORK_GLOBALFUNCTIONS_MATCHING_SERIALIZEABLEMATCHERIF_H_
#define FRAMEWORK_GLOBALFUNCTIONS_MATCHING_SERIALIZEABLEMATCHERIF_H_

#include "MatcherIF.h"
#include "../../serialize/SerializeIF.h"

template<typename T>
class SerializeableMatcherIF : public MatcherIF<T>, public SerializeIF {
public:
	virtual ~SerializeableMatcherIF() {}
};

#endif /* FRAMEWORK_GLOBALFUNCTIONS_MATCHING_SERIALIZEABLEMATCHERIF_H_ */
