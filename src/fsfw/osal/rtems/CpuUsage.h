#ifndef CPUUSAGE_H_
#define CPUUSAGE_H_

#include <stdarg.h>

#include "fsfw/container/FixedArrayList.h"
#include "fsfw/serialize/SerializeIF.h"

class CpuUsage : public SerializeIF {
 public:
  static const uint8_t MAXIMUM_NUMBER_OF_THREADS = 30;

  class ThreadData : public SerializeIF {
   public:
    static const uint8_t MAX_LENGTH_OF_THREAD_NAME = 4;

    uint32_t id;
    char name[MAX_LENGTH_OF_THREAD_NAME];
    float timeRunning;
    float percentUsage;

    virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                    Endianness streamEndianness) const override;

    virtual size_t getSerializedSize() const override;

    virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                                      Endianness streamEndianness) override;
  };

  CpuUsage();
  virtual ~CpuUsage();

  uint8_t counter;
  float timeSinceLastReset;
  FixedArrayList<ThreadData, MAXIMUM_NUMBER_OF_THREADS> threadData;
  ThreadData cachedValue;

  static void resetCpuUsage();

  void read();

  void clear();

  virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                  Endianness streamEndianness) const override;

  virtual size_t getSerializedSize() const override;

  virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                                    Endianness streamEndianness) override;
};

#endif /* CPUUSAGE_H_ */
