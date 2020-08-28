#ifndef SERIALFIXEDARRAYLISTADAPTER_H_
#define SERIALFIXEDARRAYLISTADAPTER_H_

#include "../container/FixedArrayList.h"
#include "../serialize/SerialArrayListAdapter.h"

/**
 * @brief 	This adapter provides an interface for SerializeIF to serialize and
 * 			deserialize buffers with a header containing the buffer length.
 * @details
 * Can be used by SerialLinkedListAdapter by declaring
 * as a linked element with SerializeElement<SerialFixedArrayListAdapter<...>>.
 * The sequence of objects is defined in the constructor by
 * using the setStart and setNext functions.
 *
 *  - Buffers with a size header inside that class can be declared with
 *    @code
 *    SerialFixedArrayListAdapter<BUFFER_TYPE,
 *          MAX_SIZE, count_t> mySerialFixedArrayList(...).
 *    @endcode
 *
 *  - MAX_SIZE: specifies the maximum allowed number of elements
 *    in FixedArrayList.
 *  - BUFFER_TYPE: specifies the data type of the buffer
 *  - count_t: specifies the type/size of the length field
 *    which defaults to one byte.
 *
 * @ingroup serialize
 */
template<typename BUFFER_TYPE, uint32_t MAX_SIZE, typename count_t = uint8_t>
class SerialFixedArrayListAdapter :
		public FixedArrayList<BUFFER_TYPE, MAX_SIZE, count_t>,
		public SerializeIF {
public:
	/**
	 * Constructor Arguments are forwarded to FixedArrayList constructor.
	 * Refer to the fixed array list constructors for different options.
	 * @param args
	 */
	template<typename... Args>
	SerialFixedArrayListAdapter(Args... args) :
	        FixedArrayList<BUFFER_TYPE, MAX_SIZE, count_t>(
	        std::forward<Args>(args)...){}

    ReturnValue_t serialize(uint8_t** buffer, size_t* size,
            size_t maxSize, Endianness streamEndianness) const {
        return SerialArrayListAdapter<BUFFER_TYPE, count_t>::serialize(this,
                buffer, size, maxSize, streamEndianness);
    }

    size_t getSerializedSize() const {
        return SerialArrayListAdapter<BUFFER_TYPE, count_t>::getSerializedSize(this);
    }

    ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
            Endianness streamEndianness) {
        return SerialArrayListAdapter<BUFFER_TYPE, count_t>::deSerialize(this,
                buffer, size, streamEndianness);
    }

};

#endif /* SERIALFIXEDARRAYLISTADAPTER_H_ */
