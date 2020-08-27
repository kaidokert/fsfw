#ifndef FIXEDARRAYLIST_H_
#define FIXEDARRAYLIST_H_

#include "ArrayList.h"
/**
 * \ingroup container
 */
template<typename T, uint32_t MAX_SIZE, typename count_t = uint8_t>
class FixedArrayList: public ArrayList<T, count_t> {
private:
	T data[MAX_SIZE];
public:
	FixedArrayList() :
			ArrayList<T, count_t>(data, MAX_SIZE) {
	}

	FixedArrayList(const FixedArrayList& other) :
			ArrayList<T, count_t>(data, MAX_SIZE) {
		memcpy(this->data, other.data, sizeof(this->data));
		this->entries = data;
	}

	FixedArrayList& operator=(FixedArrayList other) {
		memcpy(this->data, other.data, sizeof(this->data));
		this->entries = data;
		return *this;
	}

	virtual ~FixedArrayList() {
	}

};

#endif /* FIXEDARRAYLIST_H_ */
