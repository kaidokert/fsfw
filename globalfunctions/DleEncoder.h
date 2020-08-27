#ifndef DLEENCODER_H_
#define DLEENCODER_H_

#include "../returnvalues/HasReturnvaluesIF.h"

class DleEncoder: public HasReturnvaluesIF {
private:
	DleEncoder();
	virtual ~DleEncoder();

public:
	static const uint8_t STX = 0x02;
	static const uint8_t ETX = 0x03;
	static const uint8_t DLE = 0x10;

	static ReturnValue_t decode(const uint8_t *sourceStream,
			uint32_t sourceStreamLen, uint32_t *readLen, uint8_t *destStream,
			uint32_t maxDestStreamlen, uint32_t *decodedLen);

	static ReturnValue_t encode(const uint8_t *sourceStream, uint32_t sourceLen,
			uint8_t *destStream, uint32_t maxDestLen, uint32_t *encodedLen,
			bool addStxEtx = true);
};

#endif /* DLEENCODER_H_ */
