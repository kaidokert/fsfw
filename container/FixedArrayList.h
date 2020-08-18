#ifndef FIXEDARRAYLIST_H_
#define FIXEDARRAYLIST_H_

#include "../container/ArrayList.h"

/**
 * @brief 	Array List with a fixed maximum size
 * @ingroup container
 */
template<typename T, uint32_t MAX_SIZE, typename count_t = uint8_t>
class FixedArrayList: public ArrayList<T, count_t> {
private:
	T data[MAX_SIZE];
public:
	/**
	 * (Robin) Maybe we should also implement move assignment and move ctor.
	 * Or at least delete them.
	 */
	FixedArrayList() :
			ArrayList<T, count_t>(data, MAX_SIZE) {
	}

	// (Robin): We could create a constructor to initialize the fixed array list
	// with data and the known size field
	// so it can be used for serialization too (with SerialFixedArrrayListAdapter)
	// is this feasible?
	/**
	 * Initialize a fixed array list with data and number of data fields.
	 * Endianness of entries can be swapped optionally.
	 * @param data_
	 * @param count
	 * @param swapArrayListEndianess
	 */
	FixedArrayList(T * data_, count_t count):
		ArrayList<T, count_t>(data, MAX_SIZE) {
		memcpy(this->data, data_, count * sizeof(T));
		this->size = count;
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
