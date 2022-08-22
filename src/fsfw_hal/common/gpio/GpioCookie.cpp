#include "fsfw_hal/common/gpio/GpioCookie.h"

#include "fsfw/serviceinterface/ServiceInterface.h"

GpioCookie::GpioCookie() {}

ReturnValue_t GpioCookie::addGpio(gpioId_t gpioId, GpioBase* gpioConfig) {
  if (gpioConfig == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "GpioCookie::addGpio: gpioConfig is nullpointer" << std::endl;
#else
    sif::printWarning("GpioCookie::addGpio: gpioConfig is nullpointer\n");
#endif
    return returnvalue::FAILED;
  }
  auto gpioMapIter = gpioMap.find(gpioId);
  if (gpioMapIter == gpioMap.end()) {
    auto statusPair = gpioMap.emplace(gpioId, gpioConfig);
    if (statusPair.second == false) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "GpioCookie::addGpio: Failed to add GPIO " << gpioId << " to GPIO map"
                   << std::endl;
#else
      sif::printWarning("GpioCookie::addGpio: Failed to add GPIO %d to GPIO map\n", gpioId);
#endif
#endif
      return returnvalue::FAILED;
    }
    return returnvalue::OK;
  }
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "GpioCookie::addGpio: GPIO already exists in GPIO map " << std::endl;
#else
  sif::printWarning("GpioCookie::addGpio: GPIO already exists in GPIO map\n");
#endif
#endif
  return returnvalue::FAILED;
}

GpioMap GpioCookie::getGpioMap() const { return gpioMap; }

GpioCookie::~GpioCookie() {
  for (auto& config : gpioMap) {
    delete (config.second);
  }
}
