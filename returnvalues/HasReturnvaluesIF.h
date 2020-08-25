#ifndef FRAMEWORK_RETURNVALUES_HASRETURNVALUESIF_H_
#define FRAMEWORK_RETURNVALUES_HASRETURNVALUESIF_H_

#include "FwClassIds.h"
#include <config/returnvalues/classIds.h>
#include <cstdint>

#define MAKE_RETURN_CODE( number )	((INTERFACE_ID << 8) + (number))
typedef uint16_t ReturnValue_t;


class HasReturnvaluesIF {
public:
	static const ReturnValue_t RETURN_OK = 0;
	static const ReturnValue_t RETURN_FAILED = 1;
	virtual ~HasReturnvaluesIF() {}

	static ReturnValue_t makeReturnCode(uint8_t interfaceId, uint8_t number) {
	    return (interfaceId << 8) + number;
	}
};

#endif /* FRAMEWORK_RETURNVALUES_HASRETURNVALUESIF_H_ */
