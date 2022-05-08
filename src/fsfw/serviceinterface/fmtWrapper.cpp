#include "fmtWrapper.h"

#include "fsfw/ipc/MutexFactory.h"

std::array<char, 524> sif::PRINT_BUF = {};
MutexIF* sif::PRINT_MUTEX = nullptr;

const char* sif::PREFIX_ARR[4]= {DEBUG_PREFIX, INFO_PREFIX, WARNING_PREFIX, ERROR_PREFIX};

ReturnValue_t sif::initialize() {
  sif::PRINT_MUTEX = MutexFactory::instance()->createMutex();
  if(sif::PRINT_MUTEX == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

size_t sif::writeTypePrefix(LogLevel level) {
  auto idx = static_cast<unsigned int>(level);
  const auto result =
      fmt::format_to_n(PRINT_BUF.begin(), PRINT_BUF.size() - 1,
                       fmt::runtime(fmt::format(fg(LOG_COLOR_ARR[idx]), PREFIX_ARR[idx])));
  return result.size;
}