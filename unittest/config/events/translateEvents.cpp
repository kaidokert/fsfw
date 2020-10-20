/**
 * @brief    Auto-generated event translation file. Contains 80 translations.
 * Generated on: 2020-05-02 20:13:41 
 */
#include <fsfw/unittest/config/events/translateEvents.h>

const char *TEST_EVENT_SERVICE_1_STRING = "TEST_EVENT_SERVICE_1";
const char *TEST2_STRING = "TEST2";

const char * translateEvents(Event event){
	switch((event&0xFFFF)){
		case 8000:
			return TEST_EVENT_SERVICE_1_STRING;
		case 9100:
			return TEST2_STRING;
		default:
			return "UNKNOWN_EVENT";
	}
	return 0;
}
