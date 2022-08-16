#ifndef FSFW_HAL_SRC_FSFW_HAL_LINUX_UIO_UIOMAPPER_H_
#define FSFW_HAL_SRC_FSFW_HAL_LINUX_UIO_UIOMAPPER_H_

#include <sys/mman.h>

#include <string>

#include "fsfw/returnvalues/returnvalue.h"

/**
 * @brief   Class to help opening uio device files and mapping the physical addresses into the user
 *          address space.
 *
 * @author  J. Meier
 */
class UioMapper {
 public:
  enum class Permissions : int {
    READ_ONLY = PROT_READ,
    WRITE_ONLY = PROT_WRITE,
    READ_WRITE = PROT_READ | PROT_WRITE
  };

  /**
   * @brief   Constructor
   *
   * @param uioFile   The device file of the uiO to open
   * @param uioMap    Number of memory map. Most UIO drivers have only one map which has than 0.
   */
  UioMapper(std::string uioFile, int mapNum = 0);
  virtual ~UioMapper();

  /**
   * @brief   Maps the physical address into user address space and returns the mapped address
   *
   * @address The mapped user space address
   * @permissions Specifies the read/write permissions of the address region
   */
  ReturnValue_t getMappedAdress(uint32_t** address, Permissions permissions);

 private:
  static const char UIO_PATH_PREFIX[];
  static const char MAP_SUBSTR[];
  static const char SIZE_FILE_PATH[];
  static constexpr int SIZE_HEX_STRING = 10;

  std::string uioFile;
  int mapNum = 0;

  /**
   * @brief   Reads the map size from the associated sysfs size file
   *
   * @param size  The read map size
   */
  ReturnValue_t getMapSize(size_t* size);
};

#endif /* FSFW_HAL_SRC_FSFW_HAL_LINUX_UIO_UIOMAPPER_H_ */
