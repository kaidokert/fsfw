#ifndef HASMODESEQUENCEIF_H_
#define HASMODESEQUENCEIF_H_

#include "ModeDefinitions.h"
#include "ModeSequenceMessage.h"
#include "ModeStoreIF.h"

class HasModeSequenceIF {
 public:
  virtual ~HasModeSequenceIF() {}

  virtual MessageQueueId_t getSequenceCommandQueue() const = 0;
};

#endif /* HASMODESEQUENCEIF_H_ */
