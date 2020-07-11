#ifndef FRAMEWORK_CONTAINER_FIFO_H_
#define FRAMEWORK_CONTAINER_FIFO_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/container/FIFOBase.h>
#include <array>

/**
 * @brief 	Simple First-In-First-Out data structure with size fixed at
 * 			compile time
 * @details
 * Performs no dynamic memory allocation.
 * The public interface of FIFOBase exposes the user interface for the FIFO.
 * @tparam T Entry Type
 * @tparam capacity Maximum capacity
 */
template<typename T, size_t capacity>
class FIFO: public FIFOBase<T> {
public:
	FIFO(): FIFOBase<T>(fifoArray.data(), capacity) {};

	/**
	 * @brief 	Custom copy constructor to set pointer correctly.
	 * @param other
	 */
	FIFO(const FIFO& other): FIFOBase<T>(other) {
		this->setData(fifoArray.data());
	}

private:
	std::array<T, capacity> fifoArray;
};

#endif /* FRAMEWORK_CONTAINERS_STATICFIFO_H_ */
