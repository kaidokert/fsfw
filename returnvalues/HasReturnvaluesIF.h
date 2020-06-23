#ifndef FRAMEWORK_RETURNVALUES_HASRETURNVALUESIF_H_
#define FRAMEWORK_RETURNVALUES_HASRETURNVALUESIF_H_

#include <framework/returnvalues/FwClassIds.h>
#include <config/returnvalues/classIds.h>
#include <cstdint>

#define MAKE_RETURN_CODE( number )	((INTERFACE_ID << 8) + (number))
typedef uint16_t ReturnValue_t;


class HasReturnvaluesIF {
public:
	static const ReturnValue_t RETURN_OK = 0;
	static const ReturnValue_t RETURN_FAILED = 0xFFFF;
	virtual ~HasReturnvaluesIF() {}
};

#endif /* HASRETURNVALUESIF_H_ */
