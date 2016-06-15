#ifndef CPUUSAGE_H_
#define CPUUSAGE_H_

#include <framework/container/FixedArrayList.h>
#include <framework/serialize/SerializeIF.h>
#include <stdarg.h>

class CpuUsage : public SerializeIF {
public:
	static const uint8_t MAXIMUM_NUMBER_OF_THREADS = 30;

	class ThreadData: public SerializeIF {
	public:
		static const uint8_t MAX_LENGTH_OF_THREAD_NAME = 4;

		uint32_t id;
		char name[MAX_LENGTH_OF_THREAD_NAME];
		float timeRunning;
		float percentUsage;

		virtual ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
				const uint32_t max_size, bool bigEndian) const;

		virtual uint32_t getSerializedSize() const;

		virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
				bool bigEndian);
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

	virtual ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const;

	virtual uint32_t getSerializedSize() const;

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian);
};

#endif /* CPUUSAGE_H_ */
