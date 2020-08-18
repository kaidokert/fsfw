#ifndef TYPE_H_
#define TYPE_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include "../serialize/SerializeIF.h"

class Type: public SerializeIF {
public:
	enum ActualType_t {
		UINT8_T,
		INT8_T,
		UINT16_T,
		INT16_T,
		UINT32_T,
		INT32_T,
		FLOAT,
		DOUBLE,
		UNKNOWN_TYPE
	};

	Type();

	Type(ActualType_t actualType);

	Type(const Type &type);

	Type& operator=(Type rhs);

	Type& operator=(ActualType_t actualType);

	operator ActualType_t() const;

	bool operator==(const Type &rhs);
	bool operator!=(const Type &rhs);

	uint8_t getSize() const;

	ReturnValue_t getPtcPfc(uint8_t *ptc, uint8_t *pfc) const;

	static ActualType_t getActualType(uint8_t ptc, uint8_t pfc);

	virtual ReturnValue_t serialize(uint8_t **buffer, size_t *size,
			size_t maxSize, Endianness streamEndianness) const override;

	virtual size_t getSerializedSize() const override;

	virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
			Endianness streamEndianness) override;

private:
	ActualType_t actualType;
};

template<typename T>
struct PodTypeConversion {
	static const Type::ActualType_t type = Type::UNKNOWN_TYPE;
};
template<>
struct PodTypeConversion<uint8_t> {
	static const Type::ActualType_t type = Type::UINT8_T;
};
template<>
struct PodTypeConversion<uint16_t> {
	static const Type::ActualType_t type = Type::UINT16_T;
};
template<>
struct PodTypeConversion<uint32_t> {
	static const Type::ActualType_t type = Type::UINT32_T;
};
template<>
struct PodTypeConversion<int8_t> {
	static const Type::ActualType_t type = Type::INT8_T;
};
template<>
struct PodTypeConversion<int16_t> {
	static const Type::ActualType_t type = Type::INT16_T;
};
template<>
struct PodTypeConversion<int32_t> {
	static const Type::ActualType_t type = Type::INT32_T;
};
template<>
struct PodTypeConversion<float> {
	static const Type::ActualType_t type = Type::FLOAT;
};
template<>
struct PodTypeConversion<double> {
	static const Type::ActualType_t type = Type::DOUBLE;
};

#endif /* TYPE_H_ */
