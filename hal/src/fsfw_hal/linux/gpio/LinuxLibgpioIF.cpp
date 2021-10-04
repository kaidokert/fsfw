#include "fsfw_hal/linux/gpio/LinuxLibgpioIF.h"
#include "fsfw_hal/common/gpio/gpioDefinitions.h"
#include "fsfw_hal/common/gpio/GpioCookie.h"

#include <fsfw/serviceinterface/ServiceInterface.h>

#include <utility>
#include <unistd.h>
#include <gpiod.h>

LinuxLibgpioIF::LinuxLibgpioIF(object_id_t objectId) : SystemObject(objectId) {
}

LinuxLibgpioIF::~LinuxLibgpioIF() {
    for(auto& config: gpioMap) {
        delete(config.second);
    }
}

ReturnValue_t LinuxLibgpioIF::addGpios(GpioCookie* gpioCookie) {
    ReturnValue_t result;
    if(gpioCookie == nullptr) {
        sif::error << "LinuxLibgpioIF::addGpios: Invalid cookie" << std::endl;
        return RETURN_FAILED;
    }

    GpioMap mapToAdd = gpioCookie->getGpioMap();

    /* Check whether this ID already exists in the map and remove duplicates */
    result = checkForConflicts(mapToAdd);
    if (result != RETURN_OK){
        return result;
    }

    result = configureGpios(mapToAdd);
    if (result != RETURN_OK) {
        return RETURN_FAILED;
    }

    /* Register new GPIOs in gpioMap */
    gpioMap.insert(mapToAdd.begin(), mapToAdd.end());

    return RETURN_OK;
}

ReturnValue_t LinuxLibgpioIF::configureGpios(GpioMap& mapToAdd) {
    for(auto& gpioConfig: mapToAdd) {
        auto& gpioType = gpioConfig.second->gpioType;
        switch(gpioType) {
        case(gpio::GpioTypes::NONE): {
            return GPIO_INVALID_INSTANCE;
        }
        case(gpio::GpioTypes::GPIO_REGULAR_BY_CHIP): {
            auto regularGpio = dynamic_cast<GpiodRegularByChip*>(gpioConfig.second);
            if(regularGpio == nullptr) {
                return GPIO_INVALID_INSTANCE;
            }
            configureGpioByChip(gpioConfig.first, *regularGpio);
            break;
        }
        case(gpio::GpioTypes::GPIO_REGULAR_BY_LABEL):{
            auto regularGpio = dynamic_cast<GpiodRegularByLabel*>(gpioConfig.second);
            if(regularGpio == nullptr) {
                return GPIO_INVALID_INSTANCE;
            }
            configureGpioByLabel(gpioConfig.first, *regularGpio);
            break;
        }
        case(gpio::GpioTypes::CALLBACK): {
            auto gpioCallback = dynamic_cast<GpioCallback*>(gpioConfig.second);
            if(gpioCallback->callback == nullptr) {
                return GPIO_INVALID_INSTANCE;
            }
            gpioCallback->callback(gpioConfig.first, gpio::GpioOperation::WRITE,
                    gpioCallback->initValue, gpioCallback->callbackArgs);
        }
        }
    }
    return RETURN_OK;
}

ReturnValue_t LinuxLibgpioIF::configureGpioByLabel(gpioId_t gpioId,
        GpiodRegularByLabel &gpioByLabel) {
    std::string& label = gpioByLabel.label;
    struct gpiod_chip* chip = gpiod_chip_open_by_label(label.c_str());
    if (chip == nullptr) {
        sif::warning << "LinuxLibgpioIF::configureRegularGpio: Failed to open gpio from gpio "
                << "group with label " << label << ". Gpio ID: " << gpioId << std::endl;
        return RETURN_FAILED;

    }
    std::string failOutput = "label: " + label;
    return configureRegularGpio(gpioId, gpioByLabel.gpioType, chip, gpioByLabel, failOutput);
}

ReturnValue_t LinuxLibgpioIF::configureGpioByChip(gpioId_t gpioId,
        GpiodRegularByChip &gpioByChip) {
    std::string& chipname = gpioByChip.chipname;
    struct gpiod_chip* chip = gpiod_chip_open_by_name(chipname.c_str());
    if (chip == nullptr) {
        sif::warning << "LinuxLibgpioIF::configureRegularGpio: Failed to open chip "
                << chipname << ". Gpio ID: " << gpioId << std::endl;
        return RETURN_FAILED;
    }
    std::string failOutput = "chipname: " + chipname;
    return configureRegularGpio(gpioId, gpioByChip.gpioType, chip, gpioByChip, failOutput);
}

ReturnValue_t LinuxLibgpioIF::configureRegularGpio(gpioId_t gpioId, gpio::GpioTypes gpioType,
        struct gpiod_chip* chip, GpiodRegularBase& regularGpio, std::string failOutput) {
    unsigned int lineNum;
    gpio::Direction direction;
    std::string consumer;
    struct gpiod_line *lineHandle;
    int result = 0;

    lineNum = regularGpio.lineNum;
    lineHandle = gpiod_chip_get_line(chip, lineNum);
    if (!lineHandle) {
        sif::warning << "LinuxLibgpioIF::configureRegularGpio: Failed to open line " << std::endl;
        sif::warning << "GPIO ID: " << gpioId << ", line number: " << lineNum <<
                ", " << failOutput << std::endl;
        sif::warning << "Check if Linux GPIO configuration has changed. " << std::endl;
        gpiod_chip_close(chip);
        return RETURN_FAILED;
    }

    direction = regularGpio.direction;
    consumer = regularGpio.consumer;
    /* Configure direction and add a description to the GPIO */
    switch (direction) {
    case(gpio::OUT): {
        result = gpiod_line_request_output(lineHandle, consumer.c_str(),
                regularGpio.initValue);
        if (result < 0) {
            sif::error << "LinuxLibgpioIF::configureRegularGpio: Failed to request line " << lineNum <<
                    " from GPIO instance with ID: " << gpioId << std::endl;
            gpiod_line_release(lineHandle);
            return RETURN_FAILED;
        }
        break;
    }
    case(gpio::IN): {
        result = gpiod_line_request_input(lineHandle, consumer.c_str());
        if (result < 0) {
            sif::error << "LinuxLibgpioIF::configureGpios: Failed to request line "
                    << lineNum << " from GPIO instance with ID: " << gpioId << std::endl;
            gpiod_line_release(lineHandle);
            return RETURN_FAILED;
        }
        break;
    }
    default: {
        sif::error << "LinuxLibgpioIF::configureGpios: Invalid direction specified"
                << std::endl;
        return GPIO_INVALID_INSTANCE;
    }

    }
    /**
     * Write line handle to GPIO configuration instance so it can later be used to set or
     * read states of GPIOs.
     */
    regularGpio.lineHandle = lineHandle;
    return RETURN_OK;
}

ReturnValue_t LinuxLibgpioIF::pullHigh(gpioId_t gpioId) {
    gpioMapIter = gpioMap.find(gpioId);
    if (gpioMapIter == gpioMap.end()) {
        sif::warning << "LinuxLibgpioIF::pullHigh: Unknown GPIO ID " << gpioId << std::endl;
        return UNKNOWN_GPIO_ID;
    }

    auto gpioType = gpioMapIter->second->gpioType;
    if(gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_CHIP or
            gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_LABEL) {
        auto regularGpio = dynamic_cast<GpiodRegularBase*>(gpioMapIter->second);
        if(regularGpio == nullptr) {
            return GPIO_TYPE_FAILURE;
        }
        return driveGpio(gpioId, *regularGpio, gpio::HIGH);
    }
    else {
        auto gpioCallback = dynamic_cast<GpioCallback*>(gpioMapIter->second);
        if(gpioCallback->callback == nullptr) {
            return GPIO_INVALID_INSTANCE;
        }
        gpioCallback->callback(gpioMapIter->first, gpio::GpioOperation::WRITE,
                1, gpioCallback->callbackArgs);
        return RETURN_OK;
    }
    return GPIO_TYPE_FAILURE;
}

ReturnValue_t LinuxLibgpioIF::pullLow(gpioId_t gpioId) {
    gpioMapIter = gpioMap.find(gpioId);
    if (gpioMapIter == gpioMap.end()) {
        sif::warning << "LinuxLibgpioIF::pullLow: Unknown GPIO ID " << gpioId << std::endl;
        return UNKNOWN_GPIO_ID;
    }

    auto& gpioType = gpioMapIter->second->gpioType;
    if(gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_CHIP or
            gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_LABEL) {
        auto regularGpio = dynamic_cast<GpiodRegularBase*>(gpioMapIter->second);
        if(regularGpio == nullptr) {
            return GPIO_TYPE_FAILURE;
        }
        return driveGpio(gpioId, *regularGpio, gpio::LOW);
    }
    else {
        auto gpioCallback = dynamic_cast<GpioCallback*>(gpioMapIter->second);
        if(gpioCallback->callback == nullptr) {
            return GPIO_INVALID_INSTANCE;
        }
        gpioCallback->callback(gpioMapIter->first, gpio::GpioOperation::WRITE,
                0, gpioCallback->callbackArgs);
        return RETURN_OK;
    }
    return GPIO_TYPE_FAILURE;
}

ReturnValue_t LinuxLibgpioIF::driveGpio(gpioId_t gpioId,
        GpiodRegularBase& regularGpio, gpio::Levels logicLevel) {
    int result = gpiod_line_set_value(regularGpio.lineHandle, logicLevel);
    if (result < 0) {
        sif::warning << "LinuxLibgpioIF::driveGpio: Failed to pull GPIO with ID " << gpioId <<
                " to logic level " << logicLevel << std::endl;
        return DRIVE_GPIO_FAILURE;
    }

    return RETURN_OK;
}

ReturnValue_t LinuxLibgpioIF::readGpio(gpioId_t gpioId, int* gpioState) {
    gpioMapIter = gpioMap.find(gpioId);
    if (gpioMapIter == gpioMap.end()){
        sif::warning << "LinuxLibgpioIF::readGpio: Unknown GPIOD ID " << gpioId << std::endl;
        return UNKNOWN_GPIO_ID;
    }
    auto gpioType = gpioMapIter->second->gpioType;
    if(gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_CHIP or
            gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_LABEL) {
        auto regularGpio = dynamic_cast<GpiodRegularBase*>(gpioMapIter->second);
        if(regularGpio == nullptr) {
            return GPIO_TYPE_FAILURE;
        }
        *gpioState = gpiod_line_get_value(regularGpio->lineHandle);
    }
    else {

    }


    return RETURN_OK;
}

ReturnValue_t LinuxLibgpioIF::checkForConflicts(GpioMap& mapToAdd){
    ReturnValue_t status = HasReturnvaluesIF::RETURN_OK;
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    for(auto& gpioConfig: mapToAdd) {
        switch(gpioConfig.second->gpioType) {
        case(gpio::GpioTypes::GPIO_REGULAR_BY_CHIP):
        case(gpio::GpioTypes::GPIO_REGULAR_BY_LABEL): {
            auto regularGpio = dynamic_cast<GpiodRegularBase*>(gpioConfig.second);
            if(regularGpio == nullptr)  {
                return GPIO_TYPE_FAILURE;
            }
            /* Check for conflicts and remove duplicates if necessary */
            result = checkForConflictsRegularGpio(gpioConfig.first, *regularGpio, mapToAdd);
            if(result != HasReturnvaluesIF::RETURN_OK) {
                status = result;
            }
            break;
        }
        case(gpio::GpioTypes::CALLBACK): {
            auto callbackGpio = dynamic_cast<GpioCallback*>(gpioConfig.second);
            if(callbackGpio == nullptr)  {
                return GPIO_TYPE_FAILURE;
            }
            /* Check for conflicts and remove duplicates if necessary */
            result = checkForConflictsCallbackGpio(gpioConfig.first, callbackGpio, mapToAdd);
            if(result != HasReturnvaluesIF::RETURN_OK) {
                status = result;
            }
            break;
        }
        default: {

        }
        }
    }
    return status;
}


ReturnValue_t LinuxLibgpioIF::checkForConflictsRegularGpio(gpioId_t gpioIdToCheck,
        GpiodRegularBase& gpioToCheck, GpioMap& mapToAdd) {
    /* Cross check with private map */
    gpioMapIter = gpioMap.find(gpioIdToCheck);
    if(gpioMapIter != gpioMap.end()) {
        auto& gpioType = gpioMapIter->second->gpioType;
        if(gpioType != gpio::GpioTypes::GPIO_REGULAR_BY_CHIP and
                gpioType != gpio::GpioTypes::GPIO_REGULAR_BY_LABEL) {
            sif::warning << "LinuxLibgpioIF::checkForConflicts: ID already exists for different "
                    "GPIO type" << gpioIdToCheck << ". Removing duplicate." << std::endl;
            mapToAdd.erase(gpioIdToCheck);
            return HasReturnvaluesIF::RETURN_OK;
        }
        auto ownRegularGpio = dynamic_cast<GpiodRegularBase*>(gpioMapIter->second);
        if(ownRegularGpio == nullptr) {
            return GPIO_TYPE_FAILURE;
        }

        /* Remove element from map to add because a entry for this GPIO
        already exists */
        sif::warning << "LinuxLibgpioIF::checkForConflictsRegularGpio: Duplicate GPIO definition"
                << " detected. Duplicate will be removed from map to add." << std::endl;
        mapToAdd.erase(gpioIdToCheck);
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t LinuxLibgpioIF::checkForConflictsCallbackGpio(gpioId_t gpioIdToCheck,
        GpioCallback *callbackGpio, GpioMap& mapToAdd) {
    /* Cross check with private map */
    gpioMapIter = gpioMap.find(gpioIdToCheck);
    if(gpioMapIter != gpioMap.end()) {
        if(gpioMapIter->second->gpioType != gpio::GpioTypes::CALLBACK) {
            sif::warning << "LinuxLibgpioIF::checkForConflicts: ID already exists for different "
                    "GPIO type" << gpioIdToCheck << ". Removing duplicate." << std::endl;
            mapToAdd.erase(gpioIdToCheck);
            return HasReturnvaluesIF::RETURN_OK;
        }

        /* Remove element from map to add because a entry for this GPIO
        already exists */
        sif::warning << "LinuxLibgpioIF::checkForConflictsRegularGpio: Duplicate GPIO definition"
                << " detected. Duplicate will be removed from map to add." << std::endl;
        mapToAdd.erase(gpioIdToCheck);
    }
    return HasReturnvaluesIF::RETURN_OK;
}
