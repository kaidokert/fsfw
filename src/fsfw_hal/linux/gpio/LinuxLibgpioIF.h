#ifndef LINUX_GPIO_LINUXLIBGPIOIF_H_
#define LINUX_GPIO_LINUXLIBGPIOIF_H_

#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/returnvalues/FwClassIds.h"
#include "fsfw_hal/common/gpio/GpioIF.h"

class GpioCookie;
class GpiodRegularIF;

/**
 * @brief	This class implements the GpioIF for a linux based system.
 * @details
 * This implementation is based on the libgpiod lib which requires Linux 4.8 or higher.
 * @note
 * The Petalinux SDK from Xilinx supports libgpiod since Petalinux 2019.1.
 */
class LinuxLibgpioIF : public GpioIF, public SystemObject {
 public:
  static const uint8_t gpioRetvalId = CLASS_ID::HAL_GPIO;

  static constexpr ReturnValue_t UNKNOWN_GPIO_ID = returnvalue::makeCode(gpioRetvalId, 1);
  static constexpr ReturnValue_t DRIVE_GPIO_FAILURE = returnvalue::makeCode(gpioRetvalId, 2);
  static constexpr ReturnValue_t GPIO_TYPE_FAILURE = returnvalue::makeCode(gpioRetvalId, 3);
  static constexpr ReturnValue_t GPIO_INVALID_INSTANCE = returnvalue::makeCode(gpioRetvalId, 4);
  static constexpr ReturnValue_t GPIO_DUPLICATE_DETECTED = returnvalue::makeCode(gpioRetvalId, 5);
  static constexpr ReturnValue_t GPIO_INIT_FAILED = returnvalue::makeCode(gpioRetvalId, 6);

  LinuxLibgpioIF(object_id_t objectId);
  virtual ~LinuxLibgpioIF();

  ReturnValue_t addGpios(GpioCookie* gpioCookie) override;
  ReturnValue_t pullHigh(gpioId_t gpioId) override;
  ReturnValue_t pullLow(gpioId_t gpioId) override;
  ReturnValue_t readGpio(gpioId_t gpioId, int* gpioState) override;

 private:
  static const size_t MAX_CHIPNAME_LENGTH = 11;
  static const int LINE_NOT_EXISTS = 0;
  static const int LINE_ERROR = -1;
  static const int LINE_FOUND = 1;

  // Holds the information and configuration of all used GPIOs
  GpioUnorderedMap gpioMap;
  GpioUnorderedMapIter gpioMapIter;

  /**
   * @brief	This functions drives line of a GPIO specified by the GPIO ID.
   *
   * @param gpioId	The GPIO ID of the GPIO to drive.
   * @param logiclevel	The logic level to set. O or 1.
   */
  ReturnValue_t driveGpio(gpioId_t gpioId, GpiodRegularBase& regularGpio, gpio::Levels logicLevel);

  ReturnValue_t configureGpioByLabel(gpioId_t gpioId, GpiodRegularByLabel& gpioByLabel);
  ReturnValue_t configureGpioByChip(gpioId_t gpioId, GpiodRegularByChip& gpioByChip);
  ReturnValue_t configureGpioByLineName(gpioId_t gpioId, GpiodRegularByLineName& gpioByLineName);
  ReturnValue_t configureRegularGpio(gpioId_t gpioId, struct gpiod_chip* chip,
                                     GpiodRegularBase& regularGpio, std::string failOutput);

  /**
   * @brief	This function checks if GPIOs are already registered and whether
   * 			there exists a conflict in the GPIO configuration. E.g. the
   * 			direction.
   *
   * @param mapToAdd	The GPIOs which shall be added to the gpioMap.
   *
   * @return	returnvalue::OK if successful, otherwise returnvalue::FAILED
   */
  ReturnValue_t checkForConflicts(GpioMap& mapToAdd);

  ReturnValue_t checkForConflictsById(gpioId_t gpiodId, gpio::GpioTypes type, GpioMap& mapToAdd);

  /**
   * @brief   Performs the initial configuration of all GPIOs specified in the GpioMap mapToAdd.
   */
  ReturnValue_t configureGpios(GpioMap& mapToAdd);

  void parseFindeLineResult(int result, std::string& lineName);
};

#endif /* LINUX_GPIO_LINUXLIBGPIOIF_H_ */
