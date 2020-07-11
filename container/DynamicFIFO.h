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
	DynamicFIFO(size_t maxCapacity): FIFOBase<T>(nullptr, maxCapacity),
			fifoVector(maxCapacity)  {
		// trying to pass the pointer of the uninitialized vector
		// to the FIFOBase constructor directly lead to a super evil bug.
		// So we do it like this now.
		this->setData(fifoVector.data());
	};

	/**
	 * @brief	Custom copy constructor which prevents setting the
	 * 	        underlying pointer wrong.
	 */
	DynamicFIFO(const DynamicFIFO& other): FIFOBase<T>(other),
			fifoVector(other.maxCapacity) {
		this->setData(fifoVector.data());
	}


private:
	std::vector<T> fifoVector;
};

#endif /* FRAMEWORK_CONTAINER_DYNAMICFIFO_H_ */
