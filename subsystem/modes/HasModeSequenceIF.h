#ifndef HASMODESEQUENCEIF_H_
#define HASMODESEQUENCEIF_H_

#include <framework/subsystem/modes/ModeDefinitions.h>
#include <framework/subsystem/modes/ModeSequenceMessage.h>
#include <framework/subsystem/modes/ModeStoreIF.h>


class HasModeSequenceIF {
public:
	virtual ~HasModeSequenceIF() {

	}

	virtual MessageQueueId_t getSequenceCommandQueue() const = 0;

};


#endif /* HASMODESEQUENCEIF_H_ */
