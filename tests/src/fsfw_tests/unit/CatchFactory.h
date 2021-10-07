#ifndef FSFW_CATCHFACTORY_H_
#define FSFW_CATCHFACTORY_H_

#include "tests/TestsConfig.h"
#include "fsfw/objectmanager/SystemObjectIF.h"
#include "fsfw/objectmanager/ObjectManager.h"

// TODO: It is possible to solve this more cleanly using a special class which
// is allowed to set the object IDs and has virtual functions.
#if FSFW_ADD_DEFAULT_FACTORY_FUNCTIONS == 1

namespace Factory {
	/**
	 * @brief 	Creates all SystemObject elements which are persistent
	 * 			during execution.
	 */
	void produceFrameworkObjects(void* args);
	void setStaticFrameworkObjectIds();

}

#endif /* FSFW_ADD_DEFAULT_FSFW_FACTORY == 1 */

#endif /* FSFW_CATCHFACTORY_H_ */
