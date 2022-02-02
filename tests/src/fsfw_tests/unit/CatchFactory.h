#ifndef FSFW_CATCHFACTORY_H_
#define FSFW_CATCHFACTORY_H_

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/objectmanager/SystemObjectIF.h"
#include "tests/TestsConfig.h"

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

}  // namespace Factory

#endif /* FSFW_ADD_DEFAULT_FSFW_FACTORY == 1 */

#endif /* FSFW_CATCHFACTORY_H_ */
