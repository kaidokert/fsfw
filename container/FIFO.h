#ifndef FRAMEWORK_CONTAINER_FIFO_H_
#define FRAMEWORK_CONTAINER_FIFO_H_

#include <framework/container/FIFOBase.h>
#include <vector>

/**
 * @brief 	Simple First-In-First-Out data structure. The maximum size
 * 			can be set in the constructor. THe public interface of
 * 			FIFOBase exposes the user interface for the FIFO.
 * @tparam T Entry Type
 * @tparam capacity Maximum capacity
 */
template<typename T>
class FIFO: public FIFOBase<T> {
public:
	FIFO(size_t maxCapacity): FIFOBase<T>(values.data(), maxCapacity) {
		values.reserve(maxCapacity);
		values.resize(maxCapacity);
	};

private:
	std::vector<T> values;
};

#endif /* FRAMEWORK_CONTAINER_FIFO_H_ */
