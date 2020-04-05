#ifndef RANGEMATCHER_H_
#define RANGEMATCHER_H_

#include <framework/globalfunctions/matching/SerializeableMatcherIF.h>
#include <framework/serialize/SerializeAdapter.h>


template<typename T>
class RangeMatcher: public SerializeableMatcherIF<T> {
public:
	bool inverted;
	T lowerBound;
	T upperBound;

	RangeMatcher() :
			inverted(true), lowerBound(1), upperBound(0) {
	}
	RangeMatcher(T lowerBound, T upperBound, bool inverted = false) :
			inverted(inverted), lowerBound(lowerBound), upperBound(upperBound) {
	}

	bool match(T input) {
		if (inverted) {
			return !doMatch(input);
		} else {
			return doMatch(input);
		}
	}

	ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			const size_t max_size, bool bigEndian) const {
		ReturnValue_t result = SerializeAdapter<T>::serialize(&lowerBound, buffer, size, max_size, bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		result = SerializeAdapter<T>::serialize(&upperBound, buffer, size, max_size, bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		return SerializeAdapter<bool>::serialize(&inverted, buffer, size, max_size, bigEndian);
	}

	size_t getSerializedSize() const {
		return sizeof(lowerBound) + sizeof(upperBound) + sizeof(bool);
	}

	ReturnValue_t deSerialize(const uint8_t** buffer, ssize_t* size,
			bool bigEndian) {
		ReturnValue_t result = SerializeAdapter<T>::deSerialize(&lowerBound, buffer, size, bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		result = SerializeAdapter<T>::deSerialize(&upperBound, buffer, size, bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		return SerializeAdapter<bool>::deSerialize(&inverted, buffer, size, bigEndian);
	}
protected:
	bool doMatch(T input) {
		return (input >= lowerBound) && (input <= upperBound);
	}
};

#endif /* RANGEMATCHER_H_ */
