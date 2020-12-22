#ifndef FSFW_CATCHFACTORY_H_
#define FSFW_CATCHFACTORY_H_

#include <fsfw/objectmanager/SystemObjectIF.h>

namespace Factory {
	/**
	 * @brief 	Creates all SystemObject elements which are persistent
	 * 			during execution.
	 */
	void produce();
	void setStaticFrameworkObjectIds();

}

#endif /* FSFW_CATCHFACTORY_H_ */
