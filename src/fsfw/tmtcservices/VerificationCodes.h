#ifndef VERIFICATIONCODES_H_
#define VERIFICATIONCODES_H_

#include <cstdint>

namespace tcverif {

enum VerificationFlags : uint8_t {
  NONE = 0b0000,
  ACCEPTANCE = 0b0001,
  START = 0b0010,
  PROGRESS = 0b0100,
  COMPLETION = 0b1000
};

enum Subservices : uint8_t {
  NOTHING_TO_REPORT = 0,
  ACCEPTANCE_SUCCESS = 1,
  ACCEPTANCE_FAILURE = 2,
  START_SUCCESS = 3,
  START_FAILURE = 4,
  PROGRESS_SUCCESS = 5,
  PROGRESS_FAILURE = 6,
  COMPLETION_SUCCESS = 7,
  COMPLETION_FAILURE = 8,
};

}  // namespace tcverif

#endif /* VERIFICATIONCODES_H_ */
