/*
 * SerialFixedArrayListAdapter.h
 *
 *  Created on: 22.07.2014
 *      Author: baetz
 */

#ifndef SERIALFIXEDARRAYLISTADAPTER_H_
#define SERIALFIXEDARRAYLISTADAPTER_H_

#include <framework/container/FixedArrayList.h>
#include <framework/serialize/SerialArrayListAdapter.h>

template<typename T, uint32_t MAX_SIZE, typename count_t = uint8_t>
class SerialFixedArrayListAdapter : public FixedArrayList<T, MAX_SIZE, count_t>, public SerializeIF {
public:
	template<typename... Args>
	SerialFixedArrayListAdapter(Args... args) : FixedArrayList<T, MAX_SIZE, count_t>(std::forward<Args>(args)...) {
	}
	ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const {
		return SerialArrayListAdapter<T, count_t>::serialize(this, buffer, size, max_size, bigEndian);
	}
	uint32_t getSerializedSize() const {
		return SerialArrayListAdapter<T, count_t>::getSerializedSize(this);
	}
	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		return SerialArrayListAdapter<T, count_t>::deSerialize(this, buffer, size, bigEndian);
	}
};



#endif /* SERIALFIXEDARRAYLISTADAPTER_H_ */
