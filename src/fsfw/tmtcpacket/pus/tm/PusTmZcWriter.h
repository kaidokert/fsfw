#ifndef FSFW_EXAMPLE_HOSTED_PUSTMZCWRITER_H
#define FSFW_EXAMPLE_HOSTED_PUSTMZCWRITER_H

#include "PusTmReader.h"

/**
 * This packet allows to update specific fields of a PUS TM packet where it is useful or necessary
 * to update them in a second step. Otherwise, it offers the same interface as @PusTmReader.
 *
 * Right now, this class supports updating the CCSDS Sequence Count and the Error Control.
 */
class PusTmZeroCopyWriter : public PusTmReader {
 public:
  PusTmZeroCopyWriter(TimeReaderIF& timeReader, uint8_t* data, size_t size);

  void setSequenceCount(uint16_t seqCount);
  void updateErrorControl();

 private:
};
#endif  // FSFW_EXAMPLE_HOSTED_PUSTMZCWRITER_H
