#ifndef CONFIG_DEVICES_LOGICALADDRESSES_H_
#define CONFIG_DEVICES_LOGICALADDRESSES_H_

#include <config/objects/systemObjectList.h>
#include <fsfw/devicehandlers/CookieIF.h>
#include <cstdint>

/**
 * Can be used for addresses for physical devices like I2C adresses.
 */
namespace addresses {
	/* Logical addresses have uint32_t datatype */
	enum logicalAddresses: address_t {
	};
}


#endif /* CONFIG_DEVICES_LOGICALADDRESSES_H_ */
