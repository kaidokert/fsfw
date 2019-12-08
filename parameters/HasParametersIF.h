#ifndef HASPARAMETERSIF_H_
#define HASPARAMETERSIF_H_

#include <framework/parameters/ParameterWrapper.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <stdint.h>

typedef uint32_t ParameterId_t;

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

	virtual ~HasParametersIF() {
	}

	/**
	 * Always set parameter before checking newValues!
	 *
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

#endif /* HASPARAMETERSIF_H_ */
