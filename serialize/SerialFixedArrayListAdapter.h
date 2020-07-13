#ifndef SERIALFIXEDARRAYLISTADAPTER_H_
#define SERIALFIXEDARRAYLISTADAPTER_H_

#include <framework/container/FixedArrayList.h>
#include <framework/serialize/SerialArrayListAdapter.h>

/**
 * \ingroup serialize
 */
template<typename T, uint32_t MAX_SIZE, typename count_t = uint8_t>
class SerialFixedArrayListAdapter : public FixedArrayList<T, MAX_SIZE, count_t>, public SerializeIF {
public:
	template<typename... Args>
	SerialFixedArrayListAdapter(Args... args) : FixedArrayList<T, MAX_SIZE, count_t>(std::forward<Args>(args)...) {
	}
	ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			size_t maxSize, Endianness streamEndianness) const {
		return SerialArrayListAdapter<T, count_t>::serialize(this, buffer, size, maxSize, streamEndianness);
	}
	size_t getSerializedSize() const {
		return SerialArrayListAdapter<T, count_t>::getSerializedSize(this);
	}
	ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
			Endianness streamEndianness) {
		return SerialArrayListAdapter<T, count_t>::deSerialize(this, buffer, size, streamEndianness);
	}
};



#endif /* SERIALFIXEDARRAYLISTADAPTER_H_ */
