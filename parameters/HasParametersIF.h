#ifndef FSFW_PARAMETERS_HASPARAMETERSIF_H_
#define FSFW_PARAMETERS_HASPARAMETERSIF_H_

#include "../parameters/ParameterWrapper.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include <stdint.h>

/** Each parameter is identified with a unique parameter ID */
typedef uint32_t ParameterId_t;

/**
 * @brief	This interface is used by components which have modifiable
 * 			parameters, e.g. atittude controllers
 * @details
 * Each parameter has a unique parameter ID. The first byte of the parameter
 * ID is the domain ID which can be used to identify unqiue spacecraft domains
 * (e.g. control and sensor domain in the AOCS controller).
 *
 * The second and third byte represent the matrix ID, which can represent
 * a 8-bit row and column number and the last byte...
 *
 * Yeah, it it matrix ID oder parameter ID now and is index a 16 bit number
 * of a 8 bit number now?
 */
class HasParametersIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::HAS_PARAMETERS_IF;
	static const ReturnValue_t INVALID_MATRIX_ID = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t INVALID_DOMAIN_ID = MAKE_RETURN_CODE(0x02);
	static const ReturnValue_t INVALID_VALUE = MAKE_RETURN_CODE(0x03);
	static const ReturnValue_t READ_ONLY = MAKE_RETURN_CODE(0x05);

	static uint8_t getDomain(ParameterId_t id) {
		return id >> 24;
	}

	static uint16_t getMatrixId(ParameterId_t id) {
		return id >> 8;
	}

	static uint8_t getIndex(ParameterId_t id) {
		return id;
	}

	static uint32_t getFullParameterId(uint8_t domainId, uint16_t parameterId,
			uint8_t index) {
		return (domainId << 24) + (parameterId << 8) + index;
	}

	virtual ~HasParametersIF() {}

	/**
	 * Always set parameter before checking newValues!
	 *
	 * @param domainId
	 * @param parameterId
	 * @param parameterWrapper
	 * @param newValues
	 * @param startAtIndex
	 * @return
	 */
	virtual ReturnValue_t getParameter(uint8_t domainId, uint16_t parameterId,
			ParameterWrapper *parameterWrapper,
			const ParameterWrapper *newValues, uint16_t startAtIndex) = 0;
};

#endif /* FSFW_PARAMETERS_HASPARAMETERSIF_H_ */
