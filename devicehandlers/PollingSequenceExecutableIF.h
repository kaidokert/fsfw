/*
 * PollingSequenceExecutableIF.h
 *
 *  Created on: Mar 30, 2012
 *      Author: baetz
 */

#ifndef POLLINGSEQUENCEEXECUTABLE_H_
#define POLLINGSEQUENCEEXECUTABLE_H_

//TODO clean this whole file up

//TODO maybe define a type to make it look cleaner and use it in the PST
#define SEND_WRITE_CMD 	0
#define GET_WRITE_REPLY 1
#define SEND_READ_CMD 	2
#define GET_READ_REPLY	3


#include <framework/osal/OSAL.h>

class PollingSequenceExecutableIF {
public:
	static uint32_t pollingSequenceLengthMs;
	static uint32_t payloadSequenceLengthMs;
	virtual void performInPST( uint8_t ) = 0;
	virtual ~PollingSequenceExecutableIF() {	}
};

#endif /* POLLINGSEQUENCEEXECUTABLE_H_ */
