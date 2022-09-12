#include "fsfw/timemanager/CCSDSTime.h"

#include <cinttypes>
#include <cmath>
#include <cstdio>

#include "fsfw/FSFW.h"

ReturnValue_t CCSDSTime::convertToCcsds(Ccs_seconds* to, const Clock::TimeOfDay_t* from) {
  ReturnValue_t result = checkTimeOfDay(from);
  if (result != returnvalue::OK) {
    return result;
  }

  to->pField = (CCS << 4);

  to->yearMSB = (from->year >> 8);
  to->yearLSB = from->year & 0xff;
  to->month = from->month;
  to->day = from->day;
  to->hour = from->hour;
  to->minute = from->minute;
  to->second = from->second;

  return returnvalue::OK;
}

ReturnValue_t CCSDSTime::convertToCcsds(Ccs_mseconds* to, const Clock::TimeOfDay_t* from) {
  ReturnValue_t result = checkTimeOfDay(from);
  if (result != returnvalue::OK) {
    return result;
  }

  to->pField = (CCS << 4) + 2;

  to->yearMSB = (from->year >> 8);
  to->yearLSB = from->year & 0xff;
  to->month = from->month;
  to->day = from->day;
  to->hour = from->hour;
  to->minute = from->minute;
  to->second = from->second;
  to->secondEminus2 = from->usecond / 10000;
  to->secondEminus4 = (from->usecond % 10000) / 100;

  return returnvalue::OK;
}

ReturnValue_t CCSDSTime::convertFromCcsds(Clock::TimeOfDay_t* to, const uint8_t* from,
                                          size_t length) {
  ReturnValue_t result;
  if (length > 0xFF) {
    return LENGTH_MISMATCH;
  }
  result = convertFromASCII(to, from, length);  // Try to parse it as ASCII
  if (result == returnvalue::OK) {
    return returnvalue::OK;
  }

  // Seems to be no ascii, try the other formats
  uint8_t codeIdentification = (*from >> 4);
  switch (codeIdentification) {
    case CUC_LEVEL1:  // CUC_LEVEL2 can not be converted to TimeOfDay (ToD is Level 1) <- Well, if
                      // we know the epoch, we can... <- see bug 1133
      return convertFromCUC(to, from, length);
    case CDS:
      return convertFromCDS(to, from, length);
    case CCS: {
      size_t temp = 0;
      return convertFromCCS(to, from, &temp, length);
    }

    default:
      return UNSUPPORTED_TIME_FORMAT;
  }
}

ReturnValue_t CCSDSTime::convertFromCUC(Clock::TimeOfDay_t* to, const uint8_t* from,
                                        uint8_t length) {
  return UNSUPPORTED_TIME_FORMAT;
}

ReturnValue_t CCSDSTime::convertFromCDS(Clock::TimeOfDay_t* to, const uint8_t* from,
                                        uint8_t length) {
  timeval time;
  ReturnValue_t result = convertFromCDS(&time, from, NULL, length);
  if (result != returnvalue::OK) {
    return result;
  }
  return Clock::convertTimevalToTimeOfDay(&time, to);
}

ReturnValue_t CCSDSTime::convertFromCCS(Clock::TimeOfDay_t* to, const uint8_t* from,
                                        size_t* foundLength, size_t maxLength) {
  uint8_t subsecondsLength = *from & 0b111;
  uint32_t totalLength = subsecondsLength + 8;
  if (maxLength < totalLength) {
    return LENGTH_MISMATCH;
  }

  *foundLength = totalLength;

  ReturnValue_t result = checkCcs(from, maxLength);

  if (result != returnvalue::OK) {
    return result;
  }
  // At this point we made sure that this is a valid ccs time
  const Ccs_mseconds* temp = reinterpret_cast<const Ccs_mseconds*>(from);

  to->year = (temp->yearMSB << 8) + temp->yearLSB;
  to->hour = temp->hour;
  to->minute = temp->minute;
  to->second = temp->second;

  if (temp->pField & (1 << 3)) {  // day of year variation
    uint16_t tempDayOfYear = (temp->month << 8) + temp->day;
    uint8_t tempDay = 0;
    uint8_t tempMonth = 0;
    result = convertDaysOfYear(tempDayOfYear, to->year, &tempMonth, &tempDay);
    if (result != returnvalue::OK) {
      return result;
    }
    to->month = tempMonth;
    to->day = tempDay;
  } else {
    to->month = temp->month;
    to->day = temp->day;
  }
  to->usecond = 0;
  if (subsecondsLength > 0) {
    *foundLength += 1;
    if (temp->secondEminus2 >= 100) {
      return INVALID_TIME_FORMAT;
    }
    to->usecond = temp->secondEminus2 * 10000;
  }

  if (subsecondsLength > 1) {
    *foundLength += 1;
    if (temp->secondEminus4 >= 100) {
      return INVALID_TIME_FORMAT;
    }
    to->usecond += temp->secondEminus4 * 100;
  }

  return returnvalue::OK;
}

ReturnValue_t CCSDSTime::convertFromASCII(Clock::TimeOfDay_t* to, const uint8_t* from,
                                          uint8_t length) {
  if (length < 19) {
    return returnvalue::FAILED;
  }
  // Newlib nano can't parse uint8, see SCNu8 documentation and https://sourceware.org/newlib/README
  // Suggestion: use uint16 all the time. This should work on all systems.
#if FSFW_NO_C99_IO == 1
  uint16_t year;
  uint16_t month;
  uint16_t day;
  uint16_t hour;
  uint16_t minute;
  float second;
  int count = sscanf((const char*)from,
                     "%4" SCNu16 "-%2" SCNu16 "-%2" SCNu16
                     "T%"
                     "2" SCNu16 ":%2" SCNu16 ":%fZ",
                     &year, &month, &day, &hour, &minute, &second);
  if (count == 6) {
    to->year = year;
    to->month = month;
    to->day = day;
    to->hour = hour;
    to->minute = minute;
    to->second = second;
    to->usecond = (second - floor(second)) * 1000000;
    return returnvalue::OK;
  }

  // try Code B (yyyy-ddd)
  count = sscanf((const char*)from,
                 "%4" SCNu16 "-%3" SCNu16 "T%2" SCNu16
                 ":%"
                 "2" SCNu16 ":%fZ",
                 &year, &day, &hour, &minute, &second);
  if (count == 5) {
    uint8_t tempDay;
    ReturnValue_t result = CCSDSTime::convertDaysOfYear(
        day, year, reinterpret_cast<uint8_t*>(&month), reinterpret_cast<uint8_t*>(&tempDay));
    if (result != returnvalue::OK) {
      return returnvalue::FAILED;
    }
    to->year = year;
    to->month = month;
    to->day = tempDay;
    to->hour = hour;
    to->minute = minute;
    to->second = second;
    to->usecond = (second - floor(second)) * 1000000;
    return returnvalue::OK;
  }
  // Warning: Compiler/Linker fails ambiguously if library does not implement
  // C99 I/O
#else
  uint16_t year;
  uint8_t month;
  uint16_t day;
  uint8_t hour;
  uint8_t minute;
  float second;
  // try Code A (yyyy-mm-dd)
  int count =
      sscanf((const char*)from, "%4" SCNu16 "-%2" SCNu8 "-%2" SCNu16 "T%2" SCNu8 ":%2" SCNu8 ":%fZ",
             &year, &month, &day, &hour, &minute, &second);
  if (count == 6) {
    to->year = year;
    to->month = month;
    to->day = day;
    to->hour = hour;
    to->minute = minute;
    to->second = second;
    to->usecond = (second - floor(second)) * 1000000;
    return returnvalue::OK;
  }

  // try Code B (yyyy-ddd)
  count = sscanf((const char*)from, "%4" SCNu16 "-%3" SCNu16 "T%2" SCNu8 ":%2" SCNu8 ":%fZ", &year,
                 &day, &hour, &minute, &second);
  if (count == 5) {
    uint8_t tempDay;
    ReturnValue_t result = CCSDSTime::convertDaysOfYear(day, year, &month, &tempDay);
    if (result != returnvalue::OK) {
      return returnvalue::FAILED;
    }
    to->year = year;
    to->month = month;
    to->day = tempDay;
    to->hour = hour;
    to->minute = minute;
    to->second = second;
    to->usecond = (second - floor(second)) * 1000000;
    return returnvalue::OK;
  }
#endif

  return UNSUPPORTED_TIME_FORMAT;
}

ReturnValue_t CCSDSTime::checkCcs(const uint8_t* time, uint8_t length) {
  const Ccs_mseconds* time_struct = reinterpret_cast<const Ccs_mseconds*>(time);

  uint8_t additionalBytes = time_struct->pField & 0b111;
  if ((additionalBytes == 0b111) || (length < (additionalBytes + 8))) {
    return INVALID_TIME_FORMAT;
  }

  if (time_struct->pField & (1 << 3)) {  // day of year variation
    uint16_t day = (time_struct->month << 8) + time_struct->day;
    if (day > 366) {
      return INVALID_TIME_FORMAT;
    }
  } else {
    if (time_struct->month > 12) {
      return INVALID_TIME_FORMAT;
    }
    if (time_struct->day > 31) {
      return INVALID_TIME_FORMAT;
    }
  }
  if (time_struct->hour > 23) {
    return INVALID_TIME_FORMAT;
  }
  if (time_struct->minute > 59) {
    return INVALID_TIME_FORMAT;
  }
  if (time_struct->second > 59) {
    return INVALID_TIME_FORMAT;
  }

  const uint8_t* additionalByte = &time_struct->secondEminus2;

  for (; additionalBytes != 0; additionalBytes--) {
    if (*additionalByte++ > 99) {
      return INVALID_TIME_FORMAT;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t CCSDSTime::convertDaysOfYear(uint16_t dayofYear, uint16_t year, uint8_t* month,
                                           uint8_t* day) {
  if (isLeapYear(year)) {
    if (dayofYear > 366) {
      return INVALID_DAY_OF_YEAR;
    }
  } else {
    if (dayofYear > 365) {
      return INVALID_DAY_OF_YEAR;
    }
  }
  *month = 1;
  if (dayofYear <= 31) {
    *day = dayofYear;
    return returnvalue::OK;
  }
  *month += 1;
  dayofYear -= 31;
  if (isLeapYear(year)) {
    if (dayofYear <= 29) {
      *day = dayofYear;
      return returnvalue::OK;
    }
    *month += 1;
    dayofYear -= 29;
  } else {
    if (dayofYear <= 28) {
      *day = dayofYear;
      return returnvalue::OK;
    }
    *month += 1;
    dayofYear -= 28;
  }
  while (*month <= 12) {
    if (dayofYear <= 31) {
      *day = dayofYear;
      return returnvalue::OK;
    }
    *month += 1;
    dayofYear -= 31;

    if (*month == 8) {
      continue;
    }

    if (dayofYear <= 30) {
      *day = dayofYear;
      return returnvalue::OK;
    }
    *month += 1;
    dayofYear -= 30;
  }
  return INVALID_DAY_OF_YEAR;
}

bool CCSDSTime::isLeapYear(uint32_t year) {
  if ((year % 400) == 0) {
    return true;
  }
  if ((year % 100) == 0) {
    return false;
  }
  if ((year % 4) == 0) {
    return true;
  }
  return false;
}

ReturnValue_t CCSDSTime::convertToCcsds(CDS_short* to, const timeval* from) {
  to->pField = (CDS << 4) + 0;
  uint32_t days = ((from->tv_sec) / SECONDS_PER_DAY) + DAYS_CCSDS_TO_UNIX_EPOCH;
  if (days > (1 << 16)) {
    // Date is beyond year 2137
    return TIME_DOES_NOT_FIT_FORMAT;
  }
  to->dayMSB = (days & 0xFF00) >> 8;
  to->dayLSB = (days & 0xFF);
  uint32_t msDay = ((from->tv_sec % SECONDS_PER_DAY) * 1000) + (from->tv_usec / 1000);
  to->msDay_hh = (msDay & 0xFF000000) >> 24;
  to->msDay_h = (msDay & 0xFF0000) >> 16;
  to->msDay_l = (msDay & 0xFF00) >> 8;
  to->msDay_ll = (msDay & 0xFF);
  return returnvalue::OK;
}

ReturnValue_t CCSDSTime::convertToCcsds(OBT_FLP* to, const timeval* from) {
  to->pFiled = (AGENCY_DEFINED << 4) + 5;
  to->seconds_hh = (from->tv_sec >> 24) & 0xff;
  to->seconds_h = (from->tv_sec >> 16) & 0xff;
  to->seconds_l = (from->tv_sec >> 8) & 0xff;
  to->seconds_ll = (from->tv_sec >> 0) & 0xff;

  // convert the µs to 2E-16 seconds
  uint64_t temp = from->tv_usec;
  temp = temp << 16;
  temp = temp / 1E6;

  to->subsecondsMSB = (temp >> 8) & 0xff;
  to->subsecondsLSB = temp & 0xff;

  return returnvalue::OK;
}

ReturnValue_t CCSDSTime::convertFromCcsds(timeval* to, const uint8_t* from, size_t* foundLength,
                                          size_t maxLength) {
  if (maxLength >= 19) {
    Clock::TimeOfDay_t timeOfDay;
    /* Try to parse it as ASCII */
    ReturnValue_t result = convertFromASCII(&timeOfDay, from, maxLength);
    if (result == returnvalue::OK) {
      return Clock::convertTimeOfDayToTimeval(&timeOfDay, to);
    }
  }

  uint8_t codeIdentification = (*from >> 4);
  switch (codeIdentification) {
    /* Unsupported, as Leap second correction would have to be applied */
    case CUC_LEVEL1:
      return UNSUPPORTED_TIME_FORMAT;
    case CDS:
      return convertFromCDS(to, from, foundLength, maxLength);
    case CCS:
      return convertFromCCS(to, from, foundLength, maxLength);
    default:
      return UNSUPPORTED_TIME_FORMAT;
  }
}

ReturnValue_t CCSDSTime::convertFromCUC(timeval* to, const uint8_t* from, size_t* foundLength,
                                        size_t maxLength) {
  if (maxLength < 1) {
    return INVALID_TIME_FORMAT;
  }
  uint8_t pField = *from;
  from++;
  ReturnValue_t result = convertFromCUC(to, pField, from, foundLength, maxLength - 1);
  if (result == returnvalue::OK) {
    if (foundLength != nullptr) {
      *foundLength += 1;
    }
  }
  return result;
}

ReturnValue_t CCSDSTime::checkTimeOfDay(const Clock::TimeOfDay_t* time) {
  if ((time->month > 12) || (time->month == 0)) {
    return INVALID_TIME_FORMAT;
  }

  if (time->day == 0) {
    return INVALID_TIME_FORMAT;
  }
  switch (time->month) {
    case 2:
      if (isLeapYear(time->year)) {
        if (time->day > 29) {
          return INVALID_TIME_FORMAT;
        }
      } else {
        if (time->day > 28) {
          return INVALID_TIME_FORMAT;
        }
      }
      break;
    case 4:
    case 6:
    case 9:
    case 11:
      if (time->day > 30) {
        return INVALID_TIME_FORMAT;
      }
      break;
    default:
      if (time->day > 31) {
        return INVALID_TIME_FORMAT;
      }
      break;
  }

  if (time->hour > 23) {
    return INVALID_TIME_FORMAT;
  }

  if (time->minute > 59) {
    return INVALID_TIME_FORMAT;
  }

  if (time->second > 59) {
    return INVALID_TIME_FORMAT;
  }

  if (time->usecond > 999999) {
    return INVALID_TIME_FORMAT;
  }

  return returnvalue::OK;
}

ReturnValue_t CCSDSTime::convertFromCDS(timeval* to, const uint8_t* from, size_t* foundLength,
                                        size_t maxLength) {
  uint8_t pField = *from;
  from++;
  // Check epoch
  if (pField & 0b1000) {
    return NOT_ENOUGH_INFORMATION_FOR_TARGET_FORMAT;
  }
  // Check length
  uint8_t expectedLength = 7;  // Including p-Field.
  bool extendedDays = pField & 0b100;
  if (extendedDays) {
    expectedLength += 1;
  }
  if ((pField & 0b11) == 0b01) {
    expectedLength += 2;
  } else if ((pField & 0b11) == 0b10) {
    expectedLength += 4;
  }
  if (foundLength != nullptr) {
    *foundLength = expectedLength;
  }
  if (expectedLength > maxLength) {
    return LENGTH_MISMATCH;
  }
  // Check and count days
  uint32_t days = 0;
  if (extendedDays) {
    days = (from[0] << 16) + (from[1] << 8) + from[2];
    from += 3;
  } else {
    days = (from[0] << 8) + from[1];
    from += 2;
  }
  // Move to POSIX epoch.
  if (days <= DAYS_CCSDS_TO_UNIX_EPOCH) {
    return INVALID_TIME_FORMAT;
  }
  days -= DAYS_CCSDS_TO_UNIX_EPOCH;
  to->tv_sec = days * SECONDS_PER_DAY;
  uint32_t msDay = (from[0] << 24) + (from[1] << 16) + (from[2] << 8) + from[3];
  from += 4;
  to->tv_sec += (msDay / 1000);
  to->tv_usec = (msDay % 1000) * 1000;
  if ((pField & 0b11) == 0b01) {
    uint16_t usecs = (from[0] << 16) + from[1];
    from += 2;
    if (usecs > 999) {
      return INVALID_TIME_FORMAT;
    }
    to->tv_usec += usecs;
  } else if ((pField & 0b11) == 0b10) {
    uint32_t picosecs = (from[0] << 24) + (from[1] << 16) + (from[2] << 8) + from[3];
    from += 4;
    if (picosecs > 999999) {
      return INVALID_TIME_FORMAT;
    }
    // Not very useful.
    to->tv_usec += (picosecs / 1000);
  }
  return returnvalue::OK;
}

ReturnValue_t CCSDSTime::convertFromCDS(timeval* to, const CCSDSTime::CDS_short* from) {
  if (to == nullptr or from == nullptr) {
    return returnvalue::FAILED;
  }
  uint16_t days = (from->dayMSB << 8) + from->dayLSB;
  if (days <= DAYS_CCSDS_TO_UNIX_EPOCH) {
    return INVALID_TIME_FORMAT;
  }
  days -= DAYS_CCSDS_TO_UNIX_EPOCH;
  to->tv_sec = days * SECONDS_PER_DAY;
  uint32_t msDay =
      (from->msDay_hh << 24) + (from->msDay_h << 16) + (from->msDay_l << 8) + from->msDay_ll;
  to->tv_sec += (msDay / 1000);
  to->tv_usec = (msDay % 1000) * 1000;
  return returnvalue::OK;
}

ReturnValue_t CCSDSTime::convertFromCDS(Clock::TimeOfDay_t* to, const CCSDSTime::CDS_short* from) {
  if (to == nullptr or from == nullptr) {
    return returnvalue::FAILED;
  }
  timeval tempTimeval;
  ReturnValue_t result = convertFromCDS(&tempTimeval, from);
  if (result != returnvalue::OK) {
    return result;
  }
  return Clock::convertTimevalToTimeOfDay(&tempTimeval, to);
}

ReturnValue_t CCSDSTime::convertFromCUC(timeval* to, uint8_t pField, const uint8_t* from,
                                        size_t* foundLength, size_t maxLength) {
  uint32_t secs = 0;
  uint32_t subSeconds = 0;
  uint8_t nCoarse = ((pField & 0b1100) >> 2) + 1;
  uint8_t nFine = (pField & 0b11);
  size_t totalLength = nCoarse + nFine;
  if (foundLength != nullptr) {
    *foundLength = totalLength;
  }
  if (totalLength > maxLength) {
    return LENGTH_MISMATCH;
  }
  for (int count = nCoarse; count > 0; count--) {
    secs += *from << (count * 8 - 8);
    from++;
  }
  for (int count = nFine; count > 0; count--) {
    subSeconds += *from << (count * 8 - 8);
    from++;
  }
  // Move to POSIX epoch.
  to->tv_sec = secs;
  if (pField & 0b10000) {
    // CCSDS-Epoch
    to->tv_sec -= (DAYS_CCSDS_TO_UNIX_EPOCH * SECONDS_PER_DAY);
  }
  to->tv_usec = subsecondsToMicroseconds(subSeconds);
  return returnvalue::OK;
}

uint32_t CCSDSTime::subsecondsToMicroseconds(uint16_t subseconds) {
  uint64_t temp = (uint64_t)subseconds * 1000000 / (1 << (sizeof(subseconds) * 8));
  return temp;
}

ReturnValue_t CCSDSTime::convertFromCCS(timeval* to, const uint8_t* from, size_t* foundLength,
                                        size_t maxLength) {
  Clock::TimeOfDay_t tempTime;
  ReturnValue_t result = convertFromCCS(&tempTime, from, foundLength, maxLength);
  if (result != returnvalue::OK) {
    return result;
  }

  return Clock::convertTimeOfDayToTimeval(&tempTime, to);
}
