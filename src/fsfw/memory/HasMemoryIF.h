#ifndef HASMEMORYIF_H_
#define HASMEMORYIF_H_

#include "../returnvalues/returnvalue.h"

class HasMemoryIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::HAS_MEMORY_IF;
  static const ReturnValue_t DO_IT_MYSELF = MAKE_RETURN_CODE(1);
  static const ReturnValue_t POINTS_TO_VARIABLE = MAKE_RETURN_CODE(2);
  static const ReturnValue_t POINTS_TO_MEMORY = MAKE_RETURN_CODE(3);
  static const ReturnValue_t ACTIVITY_COMPLETED = MAKE_RETURN_CODE(4);
  static const ReturnValue_t POINTS_TO_VECTOR_UINT8 = MAKE_RETURN_CODE(5);
  static const ReturnValue_t POINTS_TO_VECTOR_UINT16 = MAKE_RETURN_CODE(6);
  static const ReturnValue_t POINTS_TO_VECTOR_UINT32 = MAKE_RETURN_CODE(7);
  static const ReturnValue_t POINTS_TO_VECTOR_FLOAT = MAKE_RETURN_CODE(8);
  static const ReturnValue_t DUMP_NOT_SUPPORTED = MAKE_RETURN_CODE(0xA0);
  static const ReturnValue_t INVALID_SIZE = MAKE_RETURN_CODE(0xE0);
  static const ReturnValue_t INVALID_ADDRESS = MAKE_RETURN_CODE(0xE1);
  static const ReturnValue_t INVALID_CONTENT = MAKE_RETURN_CODE(0xE2);
  static const ReturnValue_t UNALIGNED_ACCESS = MAKE_RETURN_CODE(0xE3);
  static const ReturnValue_t WRITE_PROTECTED = MAKE_RETURN_CODE(0xE4);
  //	static const ReturnValue_t TARGET_BUSY = MAKE_RETURN_CODE(0xE5);
  virtual ~HasMemoryIF() {}
  virtual ReturnValue_t handleMemoryLoad(uint32_t address, const uint8_t* data, uint32_t size,
                                         uint8_t** dataPointer) = 0;
  virtual ReturnValue_t handleMemoryDump(uint32_t address, uint32_t size, uint8_t** dataPointer,
                                         uint8_t* dumpTarget) = 0;
  /**
   * Sets the address of the memory, if possible.
   * startAddress is a proposal for an address, or the base address if multiple addresses are set.
   */
  virtual ReturnValue_t setAddress(uint32_t* startAddress) { return returnvalue::FAILED; }
  static bool memAccessWasSuccessful(ReturnValue_t result) {
    switch (result) {
      case DO_IT_MYSELF:
      case POINTS_TO_MEMORY:
      case POINTS_TO_VARIABLE:
      case returnvalue::OK:
      case ACTIVITY_COMPLETED:
        return true;
      default:
        return false;
    }
  }
};

#endif /* HASMEMORYIF_H_ */
