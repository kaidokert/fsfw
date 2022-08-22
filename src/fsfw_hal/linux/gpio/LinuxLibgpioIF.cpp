#include "LinuxLibgpioIF.h"

#include <gpiod.h>
#include <unistd.h>

#include <utility>

#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw_hal/common/gpio/GpioCookie.h"
#include "fsfw_hal/common/gpio/gpioDefinitions.h"

LinuxLibgpioIF::LinuxLibgpioIF(object_id_t objectId) : SystemObject(objectId) {}

LinuxLibgpioIF::~LinuxLibgpioIF() {
  for (auto& config : gpioMap) {
    delete (config.second);
  }
}

ReturnValue_t LinuxLibgpioIF::addGpios(GpioCookie* gpioCookie) {
  ReturnValue_t result;
  if (gpioCookie == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "LinuxLibgpioIF::addGpios: Invalid cookie" << std::endl;
#endif
    return returnvalue::FAILED;
  }

  GpioMap mapToAdd = gpioCookie->getGpioMap();

  /* Check whether this ID already exists in the map and remove duplicates */
  result = checkForConflicts(mapToAdd);
  if (result != returnvalue::OK) {
    return result;
  }

  result = configureGpios(mapToAdd);
  if (result != returnvalue::OK) {
    return returnvalue::FAILED;
  }

  /* Register new GPIOs in gpioMap */
  gpioMap.insert(mapToAdd.begin(), mapToAdd.end());

  return returnvalue::OK;
}

ReturnValue_t LinuxLibgpioIF::configureGpios(GpioMap& mapToAdd) {
  ReturnValue_t result = returnvalue::OK;
  for (auto& gpioConfig : mapToAdd) {
    auto& gpioType = gpioConfig.second->gpioType;
    switch (gpioType) {
      case (gpio::GpioTypes::NONE): {
        return GPIO_INVALID_INSTANCE;
      }
      case (gpio::GpioTypes::GPIO_REGULAR_BY_CHIP): {
        auto regularGpio = dynamic_cast<GpiodRegularByChip*>(gpioConfig.second);
        if (regularGpio == nullptr) {
          return GPIO_INVALID_INSTANCE;
        }
        result = configureGpioByChip(gpioConfig.first, *regularGpio);
        break;
      }
      case (gpio::GpioTypes::GPIO_REGULAR_BY_LABEL): {
        auto regularGpio = dynamic_cast<GpiodRegularByLabel*>(gpioConfig.second);
        if (regularGpio == nullptr) {
          return GPIO_INVALID_INSTANCE;
        }
        result = configureGpioByLabel(gpioConfig.first, *regularGpio);
        break;
      }
      case (gpio::GpioTypes::GPIO_REGULAR_BY_LINE_NAME): {
        auto regularGpio = dynamic_cast<GpiodRegularByLineName*>(gpioConfig.second);
        if (regularGpio == nullptr) {
          return GPIO_INVALID_INSTANCE;
        }
        result = configureGpioByLineName(gpioConfig.first, *regularGpio);
        break;
      }
      case (gpio::GpioTypes::CALLBACK): {
        auto gpioCallback = dynamic_cast<GpioCallback*>(gpioConfig.second);
        if (gpioCallback->callback == nullptr) {
          return GPIO_INVALID_INSTANCE;
        }
        gpioCallback->callback(gpioConfig.first, gpio::GpioOperation::WRITE,
                               gpioCallback->initValue, gpioCallback->callbackArgs);
      }
    }
    if (result != returnvalue::OK) {
      return GPIO_INIT_FAILED;
    }
  }
  return result;
}

ReturnValue_t LinuxLibgpioIF::configureGpioByLabel(gpioId_t gpioId,
                                                   GpiodRegularByLabel& gpioByLabel) {
  std::string& label = gpioByLabel.label;
  struct gpiod_chip* chip = gpiod_chip_open_by_label(label.c_str());
  if (chip == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "LinuxLibgpioIF::configureGpioByLabel: Failed to open gpio from gpio "
                 << "group with label " << label << ". Gpio ID: " << gpioId << std::endl;
#endif
    return returnvalue::FAILED;
  }
  std::string failOutput = "label: " + label;
  return configureRegularGpio(gpioId, chip, gpioByLabel, failOutput);
}

ReturnValue_t LinuxLibgpioIF::configureGpioByChip(gpioId_t gpioId, GpiodRegularByChip& gpioByChip) {
  std::string& chipname = gpioByChip.chipname;
  struct gpiod_chip* chip = gpiod_chip_open_by_name(chipname.c_str());
  if (chip == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "LinuxLibgpioIF::configureGpioByChip: Failed to open chip " << chipname
                 << ". Gpio ID: " << gpioId << std::endl;
#endif
    return returnvalue::FAILED;
  }
  std::string failOutput = "chipname: " + chipname;
  return configureRegularGpio(gpioId, chip, gpioByChip, failOutput);
}

ReturnValue_t LinuxLibgpioIF::configureGpioByLineName(gpioId_t gpioId,
                                                      GpiodRegularByLineName& gpioByLineName) {
  std::string& lineName = gpioByLineName.lineName;
  char chipname[MAX_CHIPNAME_LENGTH];
  unsigned int lineOffset;

  int result =
      gpiod_ctxless_find_line(lineName.c_str(), chipname, MAX_CHIPNAME_LENGTH, &lineOffset);
  if (result != LINE_FOUND) {
    parseFindeLineResult(result, lineName);
    return returnvalue::FAILED;
  }

  gpioByLineName.lineNum = static_cast<int>(lineOffset);

  struct gpiod_chip* chip = gpiod_chip_open_by_name(chipname);
  if (chip == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "LinuxLibgpioIF::configureGpioByLineName: Failed to open chip " << chipname
                 << ". <Gpio ID: " << gpioId << std::endl;
#endif
    return returnvalue::FAILED;
  }
  std::string failOutput = "line name: " + lineName;
  return configureRegularGpio(gpioId, chip, gpioByLineName, failOutput);
}

ReturnValue_t LinuxLibgpioIF::configureRegularGpio(gpioId_t gpioId, struct gpiod_chip* chip,
                                                   GpiodRegularBase& regularGpio,
                                                   std::string failOutput) {
  unsigned int lineNum;
  gpio::Direction direction;
  std::string consumer;
  struct gpiod_line* lineHandle;
  int result = 0;

  lineNum = regularGpio.lineNum;
  lineHandle = gpiod_chip_get_line(chip, lineNum);
  if (!lineHandle) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "LinuxLibgpioIF::configureRegularGpio: Failed to open line " << std::endl;
    sif::warning << "GPIO ID: " << gpioId << ", line number: " << lineNum << ", " << failOutput
                 << std::endl;
    sif::warning << "Check if Linux GPIO configuration has changed. " << std::endl;
#endif
    gpiod_chip_close(chip);
    return returnvalue::FAILED;
  }

  direction = regularGpio.direction;
  consumer = regularGpio.consumer;
  /* Configure direction and add a description to the GPIO */
  switch (direction) {
    case (gpio::Direction::OUT): {
      result = gpiod_line_request_output(lineHandle, consumer.c_str(),
                                         static_cast<int>(regularGpio.initValue));
      break;
    }
    case (gpio::Direction::IN): {
      result = gpiod_line_request_input(lineHandle, consumer.c_str());
      break;
    }
    default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "LinuxLibgpioIF::configureGpios: Invalid direction specified" << std::endl;
#endif
      return GPIO_INVALID_INSTANCE;
    }

      if (result < 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "LinuxLibgpioIF::configureRegularGpio: Failed to request line " << lineNum
                   << " from GPIO instance with ID: " << gpioId << std::endl;
#else
        sif::printError(
            "LinuxLibgpioIF::configureRegularGpio: "
            "Failed to request line %d from GPIO instance with ID: %d\n",
            lineNum, gpioId);
#endif
        gpiod_line_release(lineHandle);
        return returnvalue::FAILED;
      }
  }
  /**
   * Write line handle to GPIO configuration instance so it can later be used to set or
   * read states of GPIOs.
   */
  regularGpio.lineHandle = lineHandle;
  return returnvalue::OK;
}

ReturnValue_t LinuxLibgpioIF::pullHigh(gpioId_t gpioId) {
  gpioMapIter = gpioMap.find(gpioId);
  if (gpioMapIter == gpioMap.end()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "LinuxLibgpioIF::pullHigh: Unknown GPIO ID " << gpioId << std::endl;
#endif
    return UNKNOWN_GPIO_ID;
  }

  auto gpioType = gpioMapIter->second->gpioType;
  if (gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_CHIP or
      gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_LABEL or
      gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_LINE_NAME) {
    auto regularGpio = dynamic_cast<GpiodRegularBase*>(gpioMapIter->second);
    if (regularGpio == nullptr) {
      return GPIO_TYPE_FAILURE;
    }
    return driveGpio(gpioId, *regularGpio, gpio::Levels::HIGH);
  } else {
    auto gpioCallback = dynamic_cast<GpioCallback*>(gpioMapIter->second);
    if (gpioCallback->callback == nullptr) {
      return GPIO_INVALID_INSTANCE;
    }
    gpioCallback->callback(gpioMapIter->first, gpio::GpioOperation::WRITE, gpio::Levels::HIGH,
                           gpioCallback->callbackArgs);
    return returnvalue::OK;
  }
  return GPIO_TYPE_FAILURE;
}

ReturnValue_t LinuxLibgpioIF::pullLow(gpioId_t gpioId) {
  gpioMapIter = gpioMap.find(gpioId);
  if (gpioMapIter == gpioMap.end()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "LinuxLibgpioIF::pullLow: Unknown GPIO ID " << gpioId << std::endl;
#else
    sif::printWarning("LinuxLibgpioIF::pullLow: Unknown GPIO ID %d\n", gpioId);
#endif
    return UNKNOWN_GPIO_ID;
  }

  auto& gpioType = gpioMapIter->second->gpioType;
  if (gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_CHIP or
      gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_LABEL or
      gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_LINE_NAME) {
    auto regularGpio = dynamic_cast<GpiodRegularBase*>(gpioMapIter->second);
    if (regularGpio == nullptr) {
      return GPIO_TYPE_FAILURE;
    }
    return driveGpio(gpioId, *regularGpio, gpio::Levels::LOW);
  } else {
    auto gpioCallback = dynamic_cast<GpioCallback*>(gpioMapIter->second);
    if (gpioCallback->callback == nullptr) {
      return GPIO_INVALID_INSTANCE;
    }
    gpioCallback->callback(gpioMapIter->first, gpio::GpioOperation::WRITE, gpio::Levels::LOW,
                           gpioCallback->callbackArgs);
    return returnvalue::OK;
  }
  return GPIO_TYPE_FAILURE;
}

ReturnValue_t LinuxLibgpioIF::driveGpio(gpioId_t gpioId, GpiodRegularBase& regularGpio,
                                        gpio::Levels logicLevel) {
  int result = gpiod_line_set_value(regularGpio.lineHandle, static_cast<int>(logicLevel));
  if (result < 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "LinuxLibgpioIF::driveGpio: Failed to pull GPIO with ID " << gpioId
                 << " to logic level " << static_cast<int>(logicLevel) << std::endl;
#else
    sif::printWarning(
        "LinuxLibgpioIF::driveGpio: Failed to pull GPIO with ID %d to "
        "logic level %d\n",
        gpioId, logicLevel);
#endif
    return DRIVE_GPIO_FAILURE;
  }

  return returnvalue::OK;
}

ReturnValue_t LinuxLibgpioIF::readGpio(gpioId_t gpioId, int* gpioState) {
  gpioMapIter = gpioMap.find(gpioId);
  if (gpioMapIter == gpioMap.end()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "LinuxLibgpioIF::readGpio: Unknown GPIOD ID " << gpioId << std::endl;
#else
    sif::printWarning("LinuxLibgpioIF::readGpio: Unknown GPIOD ID %d\n", gpioId);
#endif
    return UNKNOWN_GPIO_ID;
  }

  auto gpioType = gpioMapIter->second->gpioType;
  if (gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_CHIP or
      gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_LABEL or
      gpioType == gpio::GpioTypes::GPIO_REGULAR_BY_LINE_NAME) {
    auto regularGpio = dynamic_cast<GpiodRegularBase*>(gpioMapIter->second);
    if (regularGpio == nullptr) {
      return GPIO_TYPE_FAILURE;
    }
    *gpioState = gpiod_line_get_value(regularGpio->lineHandle);
  } else {
    auto gpioCallback = dynamic_cast<GpioCallback*>(gpioMapIter->second);
    if (gpioCallback->callback == nullptr) {
      return GPIO_INVALID_INSTANCE;
    }
    gpioCallback->callback(gpioMapIter->first, gpio::GpioOperation::READ, gpio::Levels::NONE,
                           gpioCallback->callbackArgs);
    return returnvalue::OK;
  }
  return returnvalue::OK;
}

ReturnValue_t LinuxLibgpioIF::checkForConflicts(GpioMap& mapToAdd) {
  ReturnValue_t status = returnvalue::OK;
  ReturnValue_t result = returnvalue::OK;
  for (auto& gpioConfig : mapToAdd) {
    switch (gpioConfig.second->gpioType) {
      case (gpio::GpioTypes::GPIO_REGULAR_BY_CHIP):
      case (gpio::GpioTypes::GPIO_REGULAR_BY_LABEL):
      case (gpio::GpioTypes::GPIO_REGULAR_BY_LINE_NAME): {
        auto regularGpio = dynamic_cast<GpiodRegularBase*>(gpioConfig.second);
        if (regularGpio == nullptr) {
          return GPIO_TYPE_FAILURE;
        }
        // Check for conflicts and remove duplicates if necessary
        result = checkForConflictsById(gpioConfig.first, gpioConfig.second->gpioType, mapToAdd);
        if (result != returnvalue::OK) {
          status = result;
        }
        break;
      }
      case (gpio::GpioTypes::CALLBACK): {
        auto callbackGpio = dynamic_cast<GpioCallback*>(gpioConfig.second);
        if (callbackGpio == nullptr) {
          return GPIO_TYPE_FAILURE;
        }
        // Check for conflicts and remove duplicates if necessary
        result = checkForConflictsById(gpioConfig.first, gpioConfig.second->gpioType, mapToAdd);
        if (result != returnvalue::OK) {
          status = result;
        }
        break;
      }
      default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "Invalid GPIO type detected for GPIO ID " << gpioConfig.first << std::endl;
#else
        sif::printWarning("Invalid GPIO type detected for GPIO ID %d\n", gpioConfig.first);
#endif
        status = GPIO_TYPE_FAILURE;
      }
    }
  }
  return status;
}

ReturnValue_t LinuxLibgpioIF::checkForConflictsById(gpioId_t gpioIdToCheck,
                                                    gpio::GpioTypes expectedType,
                                                    GpioMap& mapToAdd) {
  // Cross check with private map
  gpioMapIter = gpioMap.find(gpioIdToCheck);
  if (gpioMapIter != gpioMap.end()) {
    auto& gpioType = gpioMapIter->second->gpioType;
    bool eraseDuplicateDifferentType = false;
    switch (expectedType) {
      case (gpio::GpioTypes::NONE): {
        break;
      }
      case (gpio::GpioTypes::GPIO_REGULAR_BY_CHIP):
      case (gpio::GpioTypes::GPIO_REGULAR_BY_LABEL):
      case (gpio::GpioTypes::GPIO_REGULAR_BY_LINE_NAME): {
        if (gpioType == gpio::GpioTypes::NONE or gpioType == gpio::GpioTypes::CALLBACK) {
          eraseDuplicateDifferentType = true;
        }
        break;
      }
      case (gpio::GpioTypes::CALLBACK): {
        if (gpioType != gpio::GpioTypes::CALLBACK) {
          eraseDuplicateDifferentType = true;
        }
      }
    }
    if (eraseDuplicateDifferentType) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "LinuxLibgpioIF::checkForConflicts: ID already exists for "
                      "different GPIO type "
                   << gpioIdToCheck << ". Removing duplicate from map to add" << std::endl;
#else
      sif::printWarning(
          "LinuxLibgpioIF::checkForConflicts: ID already exists for "
          "different GPIO type %d. Removing duplicate from map to add\n",
          gpioIdToCheck);
#endif
      mapToAdd.erase(gpioIdToCheck);
      return GPIO_DUPLICATE_DETECTED;
    }

    // Remove element from map to add because a entry for this GPIO already exists
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "LinuxLibgpioIF::checkForConflictsRegularGpio: Duplicate GPIO "
                    "definition with ID "
                 << gpioIdToCheck << " detected. "
                 << "Duplicate will be removed from map to add" << std::endl;
#else
    sif::printWarning(
        "LinuxLibgpioIF::checkForConflictsRegularGpio: Duplicate GPIO definition "
        "with ID %d detected. Duplicate will be removed from map to add\n",
        gpioIdToCheck);
#endif
    mapToAdd.erase(gpioIdToCheck);
    return GPIO_DUPLICATE_DETECTED;
  }
  return returnvalue::OK;
}

void LinuxLibgpioIF::parseFindeLineResult(int result, std::string& lineName) {
  switch (result) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    case LINE_NOT_EXISTS:
    case LINE_ERROR: {
      sif::warning << "LinuxLibgpioIF::parseFindeLineResult: Line with name " << lineName
                   << " does not exist" << std::endl;
      break;
    }
    default: {
      sif::warning << "LinuxLibgpioIF::parseFindeLineResult: Unknown return code for line "
                      "with name "
                   << lineName << std::endl;
      break;
    }
#else
    case LINE_NOT_EXISTS:
    case LINE_ERROR: {
      sif::printWarning(
          "LinuxLibgpioIF::parseFindeLineResult: Line with name %s "
          "does not exist\n",
          lineName);
      break;
    }
    default: {
      sif::printWarning(
          "LinuxLibgpioIF::parseFindeLineResult: Unknown return code for line "
          "with name %s\n",
          lineName);
      break;
    }
#endif
  }
}
