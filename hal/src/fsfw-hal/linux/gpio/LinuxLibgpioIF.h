#ifndef LINUX_GPIO_LINUXLIBGPIOIF_H_
#define LINUX_GPIO_LINUXLIBGPIOIF_H_

#include "../../common/gpio/GpioIF.h"
#include <returnvalues/classIds.h>
#include <fsfw/objectmanager/SystemObject.h>

class GpioCookie;

/**
 * @brief	This class implements the GpioIF for a linux based system. The
 * 			implementation is based on the libgpiod lib which requires linux 4.8
 * 			or higher.
 * @note	The Petalinux SDK from Xilinx supports libgpiod since Petalinux
 * 			2019.1.
 */
class LinuxLibgpioIF : public GpioIF, public SystemObject {
public:

    static const uint8_t gpioRetvalId = CLASS_ID::HAL_GPIO;

    static constexpr ReturnValue_t UNKNOWN_GPIO_ID =
            HasReturnvaluesIF::makeReturnCode(gpioRetvalId, 1);
    static constexpr ReturnValue_t DRIVE_GPIO_FAILURE =
            HasReturnvaluesIF::makeReturnCode(gpioRetvalId, 2);
    static constexpr ReturnValue_t GPIO_TYPE_FAILURE =
            HasReturnvaluesIF::makeReturnCode(gpioRetvalId, 3);
    static constexpr ReturnValue_t GPIO_INVALID_INSTANCE =
            HasReturnvaluesIF::makeReturnCode(gpioRetvalId, 4);

	LinuxLibgpioIF(object_id_t objectId);
	virtual ~LinuxLibgpioIF();

	ReturnValue_t addGpios(GpioCookie* gpioCookie) override;
	ReturnValue_t pullHigh(gpioId_t gpioId) override;
	ReturnValue_t pullLow(gpioId_t gpioId) override;
	ReturnValue_t readGpio(gpioId_t gpioId, int* gpioState) override;

private:
	/* Holds the information and configuration of all used GPIOs */
	GpioUnorderedMap gpioMap;
	GpioUnorderedMapIter gpioMapIter;

	/**
	 * @brief	This functions drives line of a GPIO specified by the GPIO ID.
	 *
	 * @param gpioId	The GPIO ID of the GPIO to drive.
	 * @param logiclevel	The logic level to set. O or 1.
	 */
	ReturnValue_t driveGpio(gpioId_t gpioId, GpiodRegular* regularGpio, unsigned int logiclevel);

	ReturnValue_t configureRegularGpio(gpioId_t gpioId, GpiodRegular* regularGpio);

	/**
	 * @brief	This function checks if GPIOs are already registered and whether
	 * 			there exists a conflict in the GPIO configuration. E.g. the
	 * 			direction.
	 *
	 * @param mapToAdd	The GPIOs which shall be added to the gpioMap.
	 *
	 * @return	RETURN_OK if successful, otherwise RETURN_FAILED
	 */
	ReturnValue_t checkForConflicts(GpioMap& mapToAdd);

	ReturnValue_t checkForConflictsRegularGpio(gpioId_t gpiodId, GpiodRegular* regularGpio,
	        GpioMap& mapToAdd);
    ReturnValue_t checkForConflictsCallbackGpio(gpioId_t gpiodId, GpioCallback* regularGpio,
            GpioMap& mapToAdd);

	/**
	 * @brief   Performs the initial configuration of all GPIOs specified in the GpioMap mapToAdd.
	 */
	ReturnValue_t configureGpios(GpioMap& mapToAdd);

};

#endif /* LINUX_GPIO_LINUXLIBGPIOIF_H_ */
