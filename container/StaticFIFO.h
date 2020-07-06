#ifndef FRAMEWORK_CONTAINER_STATICFIFO_H_
#define FRAMEWORK_CONTAINER_STATICFIFO_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/container/FIFOBase.h>
#include <array>

namespace fsfw {

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
class StaticFIFO: public FIFOBase<T> {
public:
	StaticFIFO(): FIFOBase<T>(values.data(), capacity) {};

private:
	std::array<T, capacity> values;
};

}

#endif /* FRAMEWORK_CONTAINERS_STATICFIFO_H_ */
