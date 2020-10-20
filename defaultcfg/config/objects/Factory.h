#ifndef FACTORY_H_
#define FACTORY_H_

#include <fsfw/objectmanager/SystemObjectIF.h>
#include <cstddef>

namespace Factory {
    size_t calculateStorage(uint8_t numberOfPools, uint16_t* numberOfElements,
            uint16_t* sizeOfElements);
	/**
	 * @brief   Creates all SystemObject elements which are persistent
	 *          during execution.
	 */
	void produce();
	void setStaticFrameworkObjectIds();

}


#endif /* FACTORY_H_ */
