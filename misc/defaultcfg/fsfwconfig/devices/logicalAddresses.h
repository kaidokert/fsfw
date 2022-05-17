#ifndef CONFIG_DEVICES_LOGICALADDRESSES_H_
#define CONFIG_DEVICES_LOGICALADDRESSES_H_

#include <fsfw/devicehandlers/CookieIF.h>
#include <objects/systemObjectList.h>
#include <cstdint>

/**
 * Can be used for addresses for physical devices like I2C adresses.
 */
namespace addresses {
	/* Logical addresses have uint32_t datatype */
	enum LogAddr: address_t {
	};
}


#endif /* CONFIG_DEVICES_LOGICALADDRESSES_H_ */
