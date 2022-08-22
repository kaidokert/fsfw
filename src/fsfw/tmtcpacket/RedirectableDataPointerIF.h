#ifndef TMTCPACKET_PUS_TC_SETTABLEDATAPOINTERIF_H_
#define TMTCPACKET_PUS_TC_SETTABLEDATAPOINTERIF_H_

#include <cstddef>

#include "fsfw/returnvalues/returnvalue.h"

/**
 * @brief   This interface can be used for classes which store a reference to data. It allows
 *          the implementing class to redirect the data it refers too.
 */
class RedirectableDataPointerIF {
 public:
  virtual ~RedirectableDataPointerIF() = default;

  /**
   * Redirect the data pointer, but allow an implementation to change the data.
   * The default implementation also sets a read-only pointer where applicable.
   * @param dataPtr
   * @param maxSize Maximum allowed size in buffer which holds the data. Can be used for size
   *     checks if a struct is cast directly onto the data pointer to ensure that the buffer is
   *     large enough
   * @param args Any additional user arguments required to set the data pointer
   * @return
   *  - returnvalue::OK if the pointer was set successfully
   *  - returnvalue::FAILED on general error of if the maximum size is too small
   */
  virtual ReturnValue_t setData(uint8_t* dataPtr, size_t size, void* args) = 0;
  virtual ReturnValue_t setData(uint8_t* dataPtr, size_t size) {
    return setData(dataPtr, size, nullptr);
  }

 private:
};

#endif /* TMTCPACKET_PUS_TC_SETTABLEDATAPOINTERIF_H_ */
