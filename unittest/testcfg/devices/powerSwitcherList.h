/**
 * @file switcherList.h
 *
 * @date 28.11.2019
 */

#ifndef CONFIG_DEVICES_POWERSWITCHERLIST_H_
#define CONFIG_DEVICES_POWERSWITCHERLIST_H_

namespace switches {
	/* Switches are uint8_t datatype and go from 0 to 255 */
	enum switcherList {
		PCDU,
		GPS0,
		GPS1,
		DUMMY = 129
	};

}


#endif /* CONFIG_DEVICES_POWERSWITCHERLIST_H_ */
