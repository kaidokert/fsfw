#ifndef FSFW_PARAMETERS_HASPARAMETERSIF_H_
#define FSFW_PARAMETERS_HASPARAMETERSIF_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include "ParameterWrapper.h"
#include <cstdint>

/**
 * Each parameter is identified with a unique parameter ID
 * The first byte of the parameter ID will denote the domain ID.
 * The second and third byte will be the unique identifier number.
 */
using ParameterId_t = uint32_t;

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
 * Yeah, is it matrix ID or parameter ID now and is index a 16 bit number
 * of a 8 bit number now?
 */
class HasParametersIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::HAS_PARAMETERS_IF;
	static const ReturnValue_t INVALID_IDENTIFIER_ID = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t INVALID_DOMAIN_ID = MAKE_RETURN_CODE(0x02);
	static const ReturnValue_t INVALID_VALUE = MAKE_RETURN_CODE(0x03);
	static const ReturnValue_t READ_ONLY = MAKE_RETURN_CODE(0x05);

	static uint8_t getDomain(ParameterId_t id) {
		return id >> 24;
	}

	static uint8_t getUniqueIdentifierId(ParameterId_t id) {
		return id >> 16;
	}

	/**
	 * Get the index of a parameter. Please note that the index is always a
	 * linear index. For a vector, this is straightforward.
	 * For a matrix, the linear indexing run from left to right, top to bottom.
	 * @param id
	 * @return
	 */
	static uint16_t getIndex(ParameterId_t id) {
		return id;
	}

	static uint32_t getFullParameterId(uint8_t domainId,
	        uint8_t uniqueIdentifier, uint16_t linearIndex) {
		return (domainId << 24) + (uniqueIdentifier << 16) + linearIndex;
	}

	virtual ~HasParametersIF() {}

	/**
	 * Always set parameter before checking newValues!
	 *
	 * @param domainId
	 * @param parameterId
	 * @param parameterWrapper
	 * @param newValues
	 * @param startAtIndex Linear index, runs left to right, top to bottom for
	 * matrix indexes.
	 * @return
	 */
	virtual ReturnValue_t getParameter(uint8_t domainId,
	        uint16_t uniqueIdentifier, ParameterWrapper *parameterWrapper,
			const ParameterWrapper *newValues, uint16_t startAtIndex) = 0;
};

#endif /* FSFW_PARAMETERS_HASPARAMETERSIF_H_ */
