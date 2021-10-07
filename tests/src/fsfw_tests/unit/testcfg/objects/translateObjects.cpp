/** 
 * @brief	Auto-generated object translation file.
 * @details
 * Contains 69 translations.
 * Generated on: 2021-05-18 16:37:37
 */
#include "translateObjects.h"

const char *NO_OBJECT_STRING = "NO_OBJECT";

const char* translateObject(object_id_t object) {
	switch( (object & 0xFFFFFFFF) ) {
	case 0xFFFFFFFF:
		return NO_OBJECT_STRING;
	default:
		return "UNKNOWN_OBJECT";
	}
	return 0;
}
