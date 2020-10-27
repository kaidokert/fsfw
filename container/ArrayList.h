#ifndef FSFW_CONTAINER_ARRAYLIST_H_
#define FSFW_CONTAINER_ARRAYLIST_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include "../serialize/SerializeAdapter.h"
#include "../serialize/SerializeIF.h"

/**
 * @brief 	A List that stores its values in an array.
 * @details
 * The underlying storage is an array that can be allocated by the class
 * itself or supplied via ctor.
 *
 * @ingroup container
 */
template<typename T, typename count_t = uint8_t>
class ArrayList {
	template<typename U, typename count> friend class SerialArrayListAdapter;
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::ARRAY_LIST;
	static const ReturnValue_t FULL = MAKE_RETURN_CODE(0x01);

	/**
	 * This is the allocating constructor.
	 * It allocates an array of the specified size.
	 * @param maxSize
	 */
	ArrayList(count_t maxSize) :
		size(0), maxSize_(maxSize), allocated(true) {
		entries = new T[maxSize];
	}

	/**
	 * This is the non-allocating constructor
	 *
	 * It expects a pointer to an array of a certain size and initializes
	 * itself to it.
	 *
	 * @param storage the array to use as backend
	 * @param maxSize size of storage
	 * @param size size of data already present in storage
	 */
	ArrayList(T *storage, count_t maxSize, count_t size = 0) :
		size(size), entries(storage), maxSize_(maxSize), allocated(false) {
	}

	/**
	 * Copying is forbiden by declaring copy ctor and copy assignment deleted
	 * It is too ambigous in this case.
	 * (Allocate a new backend? Use the same? What to do in an modifying call?)
	 */
	ArrayList(const ArrayList& other) = delete;
	const ArrayList& operator=(const ArrayList& other) = delete;

	/**
	 * Number of Elements stored in this List
	 */
	count_t size;


	/**
	 * Destructor, if the allocating constructor was used, it deletes the array.
	 */
	virtual ~ArrayList() {
		if (allocated) {
			delete[] entries;
		}
	}

	/**
	 * An Iterator to go trough an ArrayList
	 *
	 * It stores a pointer to an element and increments the
	 * pointer when incremented itself.
	 */
	class Iterator {
	public:
		/**
		 * Empty ctor, points to NULL
		 */
		Iterator(): value(0) {}

		/**
		 * Initializes the Iterator to point to an element
		 *
		 * @param initialize
		 */
		Iterator(T *initialize) {
			value = initialize;
		}

		/**
		 * The current element the iterator points to
		 */
		T *value;

		Iterator& operator++() {
			value++;
			return *this;
		}

		Iterator operator++(int) {
			Iterator tmp(*this);
			operator++();
			return tmp;
		}

		Iterator& operator--() {
			value--;
			return *this;
		}

		Iterator operator--(int) {
			Iterator tmp(*this);
			operator--();
			return tmp;
		}

		T& operator*() {
			return *value;
		}

		const T& operator*() const {
			return *value;
		}

		T *operator->() {
			return value;
		}

		const T *operator->() const {
			return value;
		}
	};

	friend bool operator==(const ArrayList::Iterator& lhs,
			const ArrayList::Iterator& rhs) {
		return (lhs.value == rhs.value);
	}

	friend bool operator!=(const ArrayList::Iterator& lhs,
			const ArrayList::Iterator& rhs) {
		return not (lhs.value == rhs.value);
	}

	/**
	 * Iterator pointing to the first stored elmement
	 *
	 * @return Iterator to the first element
	 */
	Iterator begin() const {
		return Iterator(&entries[0]);
	}

	/**
	 * returns an Iterator pointing to the element after the last stored entry
	 *
	 * @return Iterator to the element after the last entry
	 */
	Iterator end() const {
		return Iterator(&entries[size]);
	}

	T & operator[](count_t i) const {
		return entries[i];
	}

	/**
	 * The first element
	 *
	 * @return pointer to the first stored element
	 */
	T *front() {
		return entries;
	}

	/**
	 * The last element
	 *
	 * does not return a valid pointer if called on an empty list.
	 *
	 * @return pointer to the last stored element
	 */
	T *back() {
		return &entries[size - 1];
		//Alternative solution
		//return const_cast<T*>(static_cast<const T*>(*this).back());
	}

	const T* back() const{
		return &entries[size-1];
	}

	/**
	 * The maximum number of elements this List can contain
	 *
	 * @return maximum number of elements
	 */
	size_t maxSize() const {
		return this->maxSize_;
	}

	/**
	 * Insert a new element into the list.
	 *
	 * The new element is inserted after the last stored element.
	 *
	 * @param entry
	 * @return
	 *          -@c FULL if the List is full
	 *          -@c RETURN_OK else
	 */
	ReturnValue_t insert(T entry) {
		if (size >= maxSize_) {
			return FULL;
		}
		entries[size] = entry;
		++size;
		return HasReturnvaluesIF::RETURN_OK;
	}

	/**
	 * clear the List
	 *
	 * This does not actually clear all entries, it only sets the size to 0.
	 */
	void clear() {
		size = 0;
	}

	count_t remaining() {
		return (maxSize_ - size);
	}

protected:
	/**
	 * pointer to the array in which the entries are stored
	 */
	T *entries;
	/**
	 * remembering the maximum size
	 */
	size_t maxSize_;

	/**
	 * true if the array was allocated and needs to be deleted in the destructor.
	 */
	bool allocated;
};



#endif /* FSFW_CONTAINER_ARRAYLIST_H_ */
