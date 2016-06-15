#include <framework/osal/CpuUsage.h>
#include <framework/serialize/SerialArrayListAdapter.h>
#include <framework/serialize/SerializeAdapter.h>
#include <string.h>

extern "C" {
#include <rtems/cpuuse.h>
}

int handlePrint(void * token, const char *format, ...) {
	CpuUsage *cpuUsage = (CpuUsage *) token;

	if (cpuUsage->counter == 0) {
		//header
		cpuUsage->counter++;
		return 0;
	}

	if (cpuUsage->counter % 2 == 1) {
		{
			//we can not tell when the last call is so we assume it be every uneven time
			va_list vl;
			va_start(vl, format);
			float timeSinceLastReset = va_arg(vl,uint32_t);
			uint32_t timeSinceLastResetDecimals = va_arg(vl,uint32_t);

			timeSinceLastReset = timeSinceLastReset
					+ (timeSinceLastResetDecimals / 1000.);

			cpuUsage->timeSinceLastReset = timeSinceLastReset;

			va_end(vl);
		}
		//task name and id
		va_list vl;
		va_start(vl, format);

		cpuUsage->cachedValue.id = va_arg(vl,uint32_t);
		const char *name = va_arg(vl,const char *);
		memcpy(cpuUsage->cachedValue.name, name,
				CpuUsage::ThreadData::MAX_LENGTH_OF_THREAD_NAME);

		va_end(vl);

	} else {
		//statistics
		va_list vl;
		va_start(vl, format);
		float run = va_arg(vl,uint32_t);
		uint32_t runDecimals = va_arg(vl,uint32_t);
		float percent = va_arg(vl,uint32_t);
		uint32_t percent_decimals = va_arg(vl,uint32_t);

		run = run + (runDecimals / 1000.);
		percent = percent + (percent_decimals / 1000.);

		cpuUsage->cachedValue.percentUsage = percent;
		cpuUsage->cachedValue.timeRunning = run;

		cpuUsage->threadData.insert(cpuUsage->cachedValue);

		va_end(vl);
	}
	cpuUsage->counter++;

	return 0;
}

CpuUsage::CpuUsage() :
		counter(0), timeSinceLastReset(0) {

}

CpuUsage::~CpuUsage() {

}

void CpuUsage::resetCpuUsage() {
	rtems_cpu_usage_reset();
}

void CpuUsage::read() {
	rtems_cpu_usage_report_with_plugin(this, &handlePrint);
}

void CpuUsage::clear() {
	counter = 0;
	timeSinceLastReset = 0;
	threadData.clear();
}

ReturnValue_t CpuUsage::serialize(uint8_t** buffer, uint32_t* size,
		const uint32_t max_size, bool bigEndian) const {
	ReturnValue_t result = SerializeAdapter<float>::serialize(
			&timeSinceLastReset, buffer, size, max_size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return SerialArrayListAdapter<ThreadData>::serialize(&threadData, buffer,
			size, max_size, bigEndian);
}

uint32_t CpuUsage::getSerializedSize() const {
	uint32_t size = 0;

	size += sizeof(timeSinceLastReset);
	size += SerialArrayListAdapter<ThreadData>::getSerializedSize(&threadData);

	return size;
}

ReturnValue_t CpuUsage::deSerialize(const uint8_t** buffer, int32_t* size,
		bool bigEndian) {
	ReturnValue_t result = SerializeAdapter<float>::deSerialize(
			&timeSinceLastReset, buffer, size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return SerialArrayListAdapter<ThreadData>::deSerialize(&threadData, buffer,
			size, bigEndian);
}

ReturnValue_t CpuUsage::ThreadData::serialize(uint8_t** buffer, uint32_t* size,
		const uint32_t max_size, bool bigEndian) const {
	ReturnValue_t result = SerializeAdapter<uint32_t>::serialize(&id, buffer,
			size, max_size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	if (*size + MAX_LENGTH_OF_THREAD_NAME > max_size) {
		return BUFFER_TOO_SHORT;
	}
	memcpy(*buffer, name, MAX_LENGTH_OF_THREAD_NAME);
	*size += MAX_LENGTH_OF_THREAD_NAME;
	*buffer += MAX_LENGTH_OF_THREAD_NAME;
	result = SerializeAdapter<float>::serialize(&timeRunning,
			buffer, size, max_size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = SerializeAdapter<float>::serialize(&percentUsage,
			buffer, size, max_size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

uint32_t CpuUsage::ThreadData::getSerializedSize() const {
	uint32_t size = 0;

	size += sizeof(id);
	size += MAX_LENGTH_OF_THREAD_NAME;
	size += sizeof(timeRunning);
	size += sizeof(percentUsage);

	return size;
}

ReturnValue_t CpuUsage::ThreadData::deSerialize(const uint8_t** buffer,
		int32_t* size, bool bigEndian) {
	ReturnValue_t result = SerializeAdapter<uint32_t>::deSerialize(&id, buffer,
			size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	if ((*size = *size - MAX_LENGTH_OF_THREAD_NAME) < 0) {
		return STREAM_TOO_SHORT;
	}
	memcpy(name, *buffer, MAX_LENGTH_OF_THREAD_NAME);
	*buffer -= MAX_LENGTH_OF_THREAD_NAME;
	result = SerializeAdapter<float>::deSerialize(&timeRunning,
			buffer, size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = SerializeAdapter<float>::deSerialize(&percentUsage,
			buffer, size, bigEndian);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return HasReturnvaluesIF::RETURN_OK;
}
