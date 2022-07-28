#ifndef FSFW_EXAMPLE_HOSTED_PUSTMZCWRITER_H
#define FSFW_EXAMPLE_HOSTED_PUSTMZCWRITER_H

#include "PusTmReader.h"
class PusTmZeroCopyWriter : public PusTmReader {
 public:
  PusTmZeroCopyWriter(TimeReaderIF& timeReader, uint8_t* data, size_t size);

  void setSequenceCount(uint16_t seqCount);
  void updateErrorControl();

 private:
};
#endif  // FSFW_EXAMPLE_HOSTED_PUSTMZCWRITER_H
