#ifndef FACTORY_H_
#define FACTORY_H_

#include <fsfw/objectmanager/SystemObjectIF.h>
#include <cstddef>

namespace Factory {
	/**
	 * @brief   Creates all SystemObject elements which are persistent
	 *          during execution.
	 */
	void produce();
	void setStaticFrameworkObjectIds();
}


#endif /* FACTORY_H_ */
