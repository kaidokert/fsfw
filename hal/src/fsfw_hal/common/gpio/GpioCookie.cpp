#include "fsfw_hal/common/gpio/GpioCookie.h"

#include "fsfw/serviceinterface.h"

GpioCookie::GpioCookie() = default;

ReturnValue_t GpioCookie::addGpio(gpioId_t gpioId, GpioBase* gpioConfig) {
  if (gpioConfig == nullptr) {
    FSFW_LOGW("addGpio: gpioConfig is nullpointer\n");
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  auto gpioMapIter = gpioMap.find(gpioId);
  if (gpioMapIter == gpioMap.end()) {
    auto statusPair = gpioMap.emplace(gpioId, gpioConfig);
    if (!statusPair.second) {
      FSFW_LOGW("addGpio: Failed to add GPIO {} to GPIO map\n", gpioId);
      return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
  }
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "GpioCookie::addGpio: GPIO already exists in GPIO map " << std::endl;
#else
  sif::printWarning("GpioCookie::addGpio: GPIO already exists in GPIO map\n");
#endif
#endif
  return HasReturnvaluesIF::RETURN_FAILED;
}

GpioMap GpioCookie::getGpioMap() const { return gpioMap; }

GpioCookie::~GpioCookie() {
  for (auto& config : gpioMap) {
    delete (config.second);
  }
}
