/**
 * @file 	UnitTestClass.h
 *
 * @date 	11.04.2020
 * @author 	R. Mueller
 */

#ifndef FRAMEWORK_TEST_UNITTESTCLASS_H_
#define FRAMEWORK_TEST_UNITTESTCLASS_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <map>
#include <vector>

/**
 * We could start doing basic forms of Unit Testing (without a framework, first)
 * for framework components. This could include:
 *
 *  1. TMTC Services
 *  2. Serialization tools
 *  3. Framework internal algorithms
 */

class UnitTestClass: public HasReturnvaluesIF {
public:
	UnitTestClass();
	virtual~ UnitTestClass();

	enum class TestIds {
		AUTO_SERIALIZATION_SIZE = 0,
		AUTO_SERIALIZATION_SERIALIZE = 1,
		AUTO_SERIALIZATION_DESERIALIZE = 2,
		SERIALIZATION_BUFFER_ADAPTER = 3,
		SERIALIZATION_FIXED_ARRAY_LIST_ADAPTER = 4,
		SERIALIZATION_COMBINATION = 5,
		TMTC_SERVICES ,
		MISC
	};

	/**
	 * Some function which calls all other tests
	 * @return
	 */
	ReturnValue_t perform_tests();

	ReturnValue_t test_serialization();
	ReturnValue_t test_autoserialization();
	ReturnValue_t test_serial_buffer_adapter();
private:
	uint32_t errorCounter = 0;
	TestIds current_id = TestIds::MISC;
	std::array<uint8_t, 512> test_array;

	using TestResultMap = std::map<uint32_t, ReturnValue_t>;
	using TestBuffer = std::vector<uint8_t>;
	TestResultMap testResultMap;

	// POD test values
	bool test_value_bool = true;
	uint8_t tv_uint8   {5};
	uint16_t tv_uint16 {283};
	uint32_t tv_uint32 {929221};
	uint64_t tv_uint64 {2929329429};

	int8_t tv_int8 {-16};
	int16_t tv_int16 {-829};
	int32_t tv_int32 {-2312};

	float tv_float {8.2149214};
	float tv_sfloat = {-922.2321321};
	double tv_double {9.2132142141e8};
	double tv_sdouble {-2.2421e19};

	ReturnValue_t put_error(TestIds currentId);
};


#endif /* FRAMEWORK_TEST_UNITTESTCLASS_H_ */
