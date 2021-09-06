#ifndef FSFWCONFIG_OBJECTS_FACTORY_H_
#define FSFWCONFIG_OBJECTS_FACTORY_H_

#include <fsfw/objectmanager/SystemObjectIF.h>
#include <cstddef>

namespace Factory {
	/**
	 * @brief   Creates all SystemObject elements which are persistent
	 *          during execution.
	 */
	void produceFsfwObjects();
	void setStaticFrameworkObjectIds();
}


#endif /* FSFWCONFIG_OBJECTS_FACTORY_H_ */
