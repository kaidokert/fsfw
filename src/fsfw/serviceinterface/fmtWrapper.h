#pragma once

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/compile.h>
#include <fmt/core.h>

#include <array>
#include <cstdint>

#include "fsfw/ipc/MutexIF.h"
#include "fsfw/timemanager/Clock.h"

// Takes from stackoverflow to display relative paths:
// https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
#ifdef FSFW_SOURCE_PATH_SIZE
#define __FILENAME_REL__ (((const char*)__FILE__ + SOURCE_PATH_SIZE))
#endif

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

namespace sif {

extern std::array<char, 524> PRINT_BUF;
extern MutexIF* PRINT_MUTEX;

static const char INFO_PREFIX[] = "INFO";
static const char DEBUG_PREFIX[] = "DEBUG";
static const char WARNING_PREFIX[] = "WARNING";
static const char ERROR_PREFIX[] = "ERROR";

enum class LogLevel : unsigned int { DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3 };

extern const char* PREFIX_ARR[4];

static const std::array<fmt::color, 4> LOG_COLOR_ARR = {
    fmt::color::deep_sky_blue, fmt::color::forest_green, fmt::color::orange_red, fmt::color::red};

ReturnValue_t initialize();

size_t writeTypePrefix(LogLevel level);

template <typename... T>
size_t logTraced(LogLevel level, const char* file, unsigned int line, bool timed,
                 fmt::format_string<T...> fmt, T&&... args) noexcept {
  if (PRINT_MUTEX == nullptr) {
    fmt::print("Please call sif::initialize at program startup\n");
    return 0;
  }
  try {
    auto fsfwret = PRINT_MUTEX->lockMutex();
    if (fsfwret != HasReturnvaluesIF::RETURN_OK) {
      fmt::print("ERROR | {} | Locking print mutex failed", __FILENAME__);
      return 0;
    }
    size_t bufPos = writeTypePrefix(level);
    auto currentIter = PRINT_BUF.begin() + bufPos;
    if (timed) {
      Clock::TimeOfDay_t logTime;
      Clock::getDateAndTime(&logTime);
      const auto result = fmt::format_to_n(currentIter, PRINT_BUF.size() - 1 - bufPos,
                                           " | {}[l.{}] | {:02}:{:02}:{:02}.{:03} | {}", file, line,
                                           logTime.hour, logTime.minute, logTime.second,
                                           logTime.usecond / 1000, fmt::format(fmt, args...));
      bufPos += result.size;
    } else {
      const auto result =
          fmt::format_to_n(currentIter, PRINT_BUF.size() - 1 - bufPos, " | {}[l.{}] | {}", file,
                           line, fmt::format(fmt, args...));
      bufPos += result.size;
    }
    PRINT_BUF[bufPos] = '\0';
    fmt::print(fmt::runtime(PRINT_BUF.data()));
    PRINT_MUTEX->unlockMutex();
    return bufPos;
  } catch (const fmt::v8::format_error& e) {
    fmt::print("Printing failed with error: {}\n", e.what());
    PRINT_MUTEX->unlockMutex();
    return 0;
  }
}

template <typename... T>
size_t log(LogLevel level, bool timed, fmt::format_string<T...> fmt, T&&... args) noexcept {
  if (PRINT_MUTEX == nullptr) {
    fmt::print("Please call sif::initialize at program startup\n");
    return 0;
  }
  try {
    auto fsfwret = PRINT_MUTEX->lockMutex();
    if (fsfwret != HasReturnvaluesIF::RETURN_OK) {
      fmt::print("ERROR | {} | Locking print mutex failed", __FILENAME__);
      return 0;
    }
    size_t bufPos = writeTypePrefix(level);
    auto currentIter = PRINT_BUF.begin() + bufPos;
    if (timed) {
      Clock::TimeOfDay_t logTime;
      Clock::getDateAndTime(&logTime);
      const auto result = fmt::format_to_n(
          currentIter, PRINT_BUF.size() - bufPos, " | {:02}:{:02}:{:02}.{:03} | {}", logTime.hour,
          logTime.minute, logTime.second, logTime.usecond / 1000, fmt::format(fmt, args...));
      bufPos += result.size;
    } else {
      const auto result = fmt::format_to_n(currentIter, PRINT_BUF.size() - bufPos, " | {}",
                                           fmt::format(fmt, args...));
      bufPos += result.size;
    }
    PRINT_BUF[bufPos] = '\0';
    fmt::print(fmt::runtime(PRINT_BUF.data()));
    PRINT_MUTEX->unlockMutex();
    return bufPos;
  } catch (const fmt::v8::format_error& e) {
    PRINT_MUTEX->unlockMutex();
    fmt::print("Printing failed with error: {}\n", e.what());
    return 0;
  }
}

template <typename... T>
void debug(const char* file, unsigned int line, fmt::format_string<T...> fmt,
           T&&... args) noexcept {
  logTraced(LogLevel::DEBUG, file, line, false, fmt, args...);
}

template <typename... T>
void debug_t(const char* file, unsigned int line, fmt::format_string<T...> fmt,
             T&&... args) noexcept {
  logTraced(LogLevel::DEBUG, file, line, true, fmt, args...);
}

template <typename... T>
void info_t(fmt::format_string<T...> fmt, T&&... args) {
  log(LogLevel::INFO, true, fmt, args...);
}

template <typename... T>
void info(fmt::format_string<T...> fmt, T&&... args) {
  log(LogLevel::INFO, false, fmt, args...);
}

template <typename... T>
void warning(fmt::format_string<T...> fmt, T&&... args) {
  log(LogLevel::ERROR, false, fmt, args...);
}

template <typename... T>
void warning_t(fmt::format_string<T...> fmt, T&&... args) {
  log(LogLevel::ERROR, true, fmt, args...);
}

template <typename... T>
void warning_f(const char* file, unsigned int line, fmt::format_string<T...> fmt, T&&... args) {
  logTraced(LogLevel::WARNING, file, line, false, fmt, args...);
}

template <typename... T>
void warning_ft(const char* file, unsigned int line, fmt::format_string<T...> fmt, T&&... args) {
  logTraced(LogLevel::WARNING, file, line, true, fmt, args...);
}

template <typename... T>
void error(fmt::format_string<T...> fmt, T&&... args) {
  log(LogLevel::ERROR, false, fmt, args...);
}

template <typename... T>
void error_t(fmt::format_string<T...> fmt, T&&... args) {
  log(LogLevel::ERROR, true, fmt, args...);
}

template <typename... T>
void error_f(const char* file, unsigned int line, fmt::format_string<T...> fmt, T&&... args) {
  logTraced(LogLevel::ERROR, file, line, false, fmt, args...);
}

template <typename... T>
void error_ft(const char* file, unsigned int line, fmt::format_string<T...> fmt, T&&... args) {
  logTraced(LogLevel::ERROR, file, line, true, fmt, args...);
}

}  // namespace sif

#define FSFW_LOGI(format, ...) sif::info(FMT_STRING(format), __VA_ARGS__)

#define FSFW_LOGIT(format, ...) sif::info_t(FMT_STRING(format), __VA_ARGS__)

#define FSFW_LOGD(format, ...) sif::debug(__FILENAME__, __LINE__, FMT_STRING(format), __VA_ARGS__)

#define FSFW_LOGDT(format, ...) \
  sif::debug_t(__FILENAME__, __LINE__, FMT_STRING(format), __VA_ARGS__)

#define FSFW_LOGW(format, ...) \
  sif::warning_f(__FILENAME__, __LINE__, FMT_STRING(format), __VA_ARGS__)

#define FSFW_LOGWT(format, ...) \
  sif::warning_ft(__FILENAME__, __LINE__, FMT_STRING(format), __VA_ARGS__)

#define FSFW_LOGE(format, ...) sif::error_f(__FILENAME__, __LINE__, FMT_STRING(format), __VA_ARGS__)

#define FSFW_LOGET(format, ...) \
  sif::error_ft(__FILENAME__, __LINE__, FMT_STRING(format), __VA_ARGS__)
