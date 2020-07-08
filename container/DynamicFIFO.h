#ifndef FRAMEWORK_CONTAINER_DYNAMICFIFO_H_
#define FRAMEWORK_CONTAINER_DYNAMICFIFO_H_

#include <framework/container/FIFOBase.h>
#include <vector>

/**
 * @brief 	Simple First-In-First-Out data structure. The maximum size
 * 			can be set in the constructor.
 * @details
 * The maximum capacity can be determined at run-time, so this container
 * performs dynamic memory allocation!
 * The public interface of FIFOBase exposes the user interface for the FIFO.
 * @tparam T Entry Type
 * @tparam capacity Maximum capacity
 */
template<typename T>
class DynamicFIFO: public FIFOBase<T> {
public:
	DynamicFIFO(size_t maxCapacity): FIFOBase<T>(values.data(), maxCapacity),
			values(maxCapacity) {};

private:
	std::vector<T> values;
};

#endif /* FRAMEWORK_CONTAINER_DYNAMICFIFO_H_ */
