/**
 * @file 	UnitTestClass.cpp
 *
 * @date 	11.04.2020
 * @author 	R. Mueller
 */
#include <framework/test/UnitTestClass.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/serialize/SerializeElement.h>
#include <framework/serialize/SerialBufferAdapter.h>

#include <cstdlib>

UnitTestClass::UnitTestClass() {
}

UnitTestClass::~UnitTestClass() {
}

ReturnValue_t UnitTestClass::perform_tests() {
	ReturnValue_t result = test_serialization();
	if(result != RETURN_OK) {
		return result;
	}

	return RETURN_OK;
}

ReturnValue_t UnitTestClass::test_serialization() {
	// Here, we test all serialization tools. First test basic cases.
	ReturnValue_t result = test_autoserialization();
	if(result != RETURN_OK) {
		return result;
	}
	result = test_serial_buffer_adapter();
	if(result != RETURN_OK) {
		return result;
	}
	return RETURN_OK;
}

ReturnValue_t UnitTestClass::test_autoserialization() {
	current_id = TestIds::AUTO_SERIALIZATION_SIZE;
	// Unit Test getSerializedSize
	if(AutoSerializeAdapter::
			getSerializedSize(&test_value_bool) != sizeof(test_value_bool) or
			AutoSerializeAdapter::
			getSerializedSize(&tv_uint8) != sizeof(tv_uint8) or
			AutoSerializeAdapter::
			getSerializedSize(&tv_uint16) != sizeof(tv_uint16) or
			AutoSerializeAdapter::
			getSerializedSize(&tv_uint32) != sizeof(tv_uint32) or
			AutoSerializeAdapter::
			getSerializedSize(&tv_uint64) != sizeof(tv_uint64) or
			AutoSerializeAdapter::
			getSerializedSize(&tv_int8) != sizeof(tv_int8) or
			AutoSerializeAdapter::
			getSerializedSize(&tv_double) != sizeof(tv_double) or
			AutoSerializeAdapter::
			getSerializedSize(&tv_int16) != sizeof(tv_int16) or
			AutoSerializeAdapter::
			getSerializedSize(&tv_int32) != sizeof(tv_int32) or
			AutoSerializeAdapter::
			getSerializedSize(&tv_float) != sizeof(tv_float))
	{
		return put_error(current_id);
	}

	// Unit Test AutoSerializeAdapter deserialize
	current_id = TestIds::AUTO_SERIALIZATION_SERIALIZE;

	size_t serialized_size = 0;
	uint8_t * p_array = test_array.data();

	AutoSerializeAdapter::serialize(&test_value_bool, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_uint8, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_uint16, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_uint32, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_int8, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_int16, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_int32, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_uint64, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_float, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_double, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_sfloat, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_sdouble, &p_array,
			&serialized_size, test_array.size(), false);
	// expected size is 1 + 1 + 2 + 4 + 1 + 2 + 4 + 8 + 4 + 8 + 4 + 8
	if(serialized_size != 47) {
		return put_error(current_id);
	}

	// Unit Test AutoSerializeAdapter serialize
	current_id = TestIds::AUTO_SERIALIZATION_DESERIALIZE;
	p_array = test_array.data();
	ssize_t remaining_size = serialized_size;
	AutoSerializeAdapter::deSerialize(&test_value_bool,
			const_cast<const uint8_t**>(&p_array), &remaining_size, false);
	AutoSerializeAdapter::deSerialize(&tv_uint8,
			const_cast<const uint8_t**>(&p_array), &remaining_size, false);
	AutoSerializeAdapter::deSerialize(&tv_uint16,
			const_cast<const uint8_t**>(&p_array), &remaining_size, false);
	AutoSerializeAdapter::deSerialize(&tv_uint32,
			const_cast<const uint8_t**>(&p_array), &remaining_size, false);
	AutoSerializeAdapter::deSerialize(&tv_int8,
			const_cast<const uint8_t**>(&p_array), &remaining_size, false);
	AutoSerializeAdapter::deSerialize(&tv_int16,
			const_cast<const uint8_t**>(&p_array), &remaining_size, false);
	AutoSerializeAdapter::deSerialize(&tv_int32,
			const_cast<const uint8_t**>(&p_array), &remaining_size, false);
	AutoSerializeAdapter::deSerialize(&tv_uint64,
			const_cast<const uint8_t**>(&p_array), &remaining_size, false);
	AutoSerializeAdapter::deSerialize(&tv_float,
			const_cast<const uint8_t**>(&p_array), &remaining_size, false);
	AutoSerializeAdapter::deSerialize(&tv_double,
			const_cast<const uint8_t**>(&p_array), &remaining_size, false);
	AutoSerializeAdapter::deSerialize(&tv_sfloat,
			const_cast<const uint8_t**>(&p_array), &remaining_size, false);
	AutoSerializeAdapter::deSerialize(&tv_sdouble,
			const_cast<const uint8_t**>(&p_array), &remaining_size, false);

	//	uint16_t tv_uint16 {283};
	//	uint32_t tv_uint32 {929221};
	//	uint64_t tv_uint64 {2929329429};
	//	int8_t tv_int8 {-16};
	//	int16_t tv_int16 {-829};
	//	int32_t tv_int32 {-2312};
	//	float tv_float {8.2149214};
	//	float tv_sfloat = {-922.2321321};
	//	double tv_double {9.2132142141e8};
	//	double tv_sdouble {-2.2421e19};
	if(test_value_bool != true or tv_uint8 != 5 or tv_uint16 != 283 or
			tv_uint32 != 929221 or tv_uint64 != 2929329429 or tv_int8 != -16 or
			tv_int16 != -829 or tv_int32 != -2312 or tv_float != 8.214921 or
			tv_double != 9.2132142141e8 or tv_sfloat != -922.2321321 or
			tv_sdouble != -2.2421e19)
	{
		return put_error(current_id);
	}
	return RETURN_OK;
}

ReturnValue_t UnitTestClass::test_serial_buffer_adapter() {
	current_id = TestIds::SERIALIZATION_BUFFER_ADAPTER;

	// I will skip endian swapper testing, its going to be changed anyway..
	// uint8_t tv_uint8_swapped = EndianSwapper::swap(tv_uint8);

	size_t serialized_size = 0;
	test_value_bool = true;
	uint8_t * p_array = test_array.data();
	std::array<uint8_t, 5> test_serial_buffer {5, 4, 3, 2, 1};
	SerialBufferAdapter<uint8_t> tv_serial_buffer_adapter =
			SerialBufferAdapter<uint8_t>(test_serial_buffer.data(),
					test_serial_buffer.size(), false);
	tv_uint16 = 16;

	AutoSerializeAdapter::serialize(&test_value_bool, &p_array,&serialized_size,
			test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_serial_buffer_adapter, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_uint16, &p_array, &serialized_size,
			test_array.size(), false);

	if(serialized_size != 8 or test_array[0] != true or test_array[1] != 5
			or test_array[2] != 4 or test_array[3] != 3 or test_array[4] != 2
			or test_array[5] != 1)
	{
		return put_error(current_id);
	}
	memcpy(&tv_uint16, test_array.data() + 6, sizeof(tv_uint16));
	if(tv_uint16 != 16) {
		return put_error(current_id);
	}

	// Serialize with size field
	SerialBufferAdapter<uint8_t> tv_serial_buffer_adapter2 =
			SerialBufferAdapter<uint8_t>(test_serial_buffer.data(),
					test_serial_buffer.size(), true);
	serialized_size = 0;
	p_array = test_array.data();
	AutoSerializeAdapter::serialize(&test_value_bool, &p_array,&serialized_size,
			test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_serial_buffer_adapter2, &p_array,
			&serialized_size, test_array.size(), false);
	AutoSerializeAdapter::serialize(&tv_uint16, &p_array, &serialized_size,
			test_array.size(), false);

	if(serialized_size != 9 or test_array[0] != true or test_array[1] != 5
			or test_array[2] != 5 or test_array[3] != 4 or test_array[4] != 3
			or test_array[5] != 2 or test_array[6] != 1)
	{
		return put_error(current_id);
	}
	memcpy(&tv_uint16, test_array.data() + 6, sizeof(tv_uint16));
	if(tv_uint16 != 16) {
		return put_error(current_id);
	}
	return RETURN_OK;
}

ReturnValue_t UnitTestClass::put_error(TestIds current_id) {
	error << "Unit Tester failed at test ID "
			<< static_cast<uint32_t>(current_id) << "\r\n" << std::flush;
	return RETURN_FAILED;
}
