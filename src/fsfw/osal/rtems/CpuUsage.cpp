#include "fsfw/osal/rtems/CpuUsage.h"

#include <string.h>

#include "fsfw/serialize/SerialArrayListAdapter.h"
#include "fsfw/serialize/SerializeAdapter.h"

extern "C" {
#include <rtems/cpuuse.h>
}

int handlePrint(void* token, const char* format, ...) {
  CpuUsage* cpuUsage = (CpuUsage*)token;

  if (cpuUsage->counter == 0) {
    // header
    cpuUsage->counter++;
    return 0;
  }

  if (cpuUsage->counter % 2 == 1) {
    {
      // we can not tell when the last call is so we assume it be every uneven time
      va_list vl;
      va_start(vl, format);
      float timeSinceLastReset = va_arg(vl, uint32_t);
      uint32_t timeSinceLastResetDecimals = va_arg(vl, uint32_t);

      timeSinceLastReset = timeSinceLastReset + (timeSinceLastResetDecimals / 1000.);

      cpuUsage->timeSinceLastReset = timeSinceLastReset;

      va_end(vl);
    }
    // task name and id
    va_list vl;
    va_start(vl, format);

    cpuUsage->cachedValue.id = va_arg(vl, uint32_t);
    const char* name = va_arg(vl, const char*);
    memcpy(cpuUsage->cachedValue.name, name, CpuUsage::ThreadData::MAX_LENGTH_OF_THREAD_NAME);

    va_end(vl);

  } else {
    // statistics
    va_list vl;
    va_start(vl, format);
    float run = va_arg(vl, uint32_t);
    uint32_t runDecimals = va_arg(vl, uint32_t);
    float percent = va_arg(vl, uint32_t);
    uint32_t percent_decimals = va_arg(vl, uint32_t);

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

CpuUsage::CpuUsage() : counter(0), timeSinceLastReset(0) {}

CpuUsage::~CpuUsage() {}

void CpuUsage::resetCpuUsage() { rtems_cpu_usage_reset(); }

void CpuUsage::read() {
  // rtems_cpu_usage_report_with_plugin(this, &handlePrint);
}

void CpuUsage::clear() {
  counter = 0;
  timeSinceLastReset = 0;
  threadData.clear();
}

ReturnValue_t CpuUsage::serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                  Endianness streamEndianness) const {
  ReturnValue_t result =
      SerializeAdapter::serialize(&timeSinceLastReset, buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  return SerialArrayListAdapter<ThreadData>::serialize(&threadData, buffer, size, maxSize,
                                                       streamEndianness);
}

uint32_t CpuUsage::getSerializedSize() const {
  uint32_t size = 0;

  size += sizeof(timeSinceLastReset);
  size += SerialArrayListAdapter<ThreadData>::getSerializedSize(&threadData);

  return size;
}

ReturnValue_t CpuUsage::deSerialize(const uint8_t** buffer, size_t* size,
                                    Endianness streamEndianness) {
  ReturnValue_t result =
      SerializeAdapter::deSerialize(&timeSinceLastReset, buffer, size, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  return SerialArrayListAdapter<ThreadData>::deSerialize(&threadData, buffer, size,
                                                         streamEndianness);
}

ReturnValue_t CpuUsage::ThreadData::serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                              Endianness streamEndianness) const {
  ReturnValue_t result = SerializeAdapter::serialize(&id, buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  if (*size + MAX_LENGTH_OF_THREAD_NAME > maxSize) {
    return BUFFER_TOO_SHORT;
  }
  memcpy(*buffer, name, MAX_LENGTH_OF_THREAD_NAME);
  *size += MAX_LENGTH_OF_THREAD_NAME;
  *buffer += MAX_LENGTH_OF_THREAD_NAME;
  result = SerializeAdapter::serialize(&timeRunning, buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  result = SerializeAdapter::serialize(&percentUsage, buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  return returnvalue::OK;
}

uint32_t CpuUsage::ThreadData::getSerializedSize() const {
  uint32_t size = 0;

  size += sizeof(id);
  size += MAX_LENGTH_OF_THREAD_NAME;
  size += sizeof(timeRunning);
  size += sizeof(percentUsage);

  return size;
}

ReturnValue_t CpuUsage::ThreadData::deSerialize(const uint8_t** buffer, size_t* size,
                                                Endianness streamEndianness) {
  ReturnValue_t result = SerializeAdapter::deSerialize(&id, buffer, size, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  if (*size < MAX_LENGTH_OF_THREAD_NAME) {
    return STREAM_TOO_SHORT;
  }
  memcpy(name, *buffer, MAX_LENGTH_OF_THREAD_NAME);
  *buffer -= MAX_LENGTH_OF_THREAD_NAME;
  result = SerializeAdapter::deSerialize(&timeRunning, buffer, size, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  result = SerializeAdapter::deSerialize(&percentUsage, buffer, size, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  return returnvalue::OK;
}
