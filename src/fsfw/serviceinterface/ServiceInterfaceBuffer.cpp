#include "fsfw/serviceinterface/ServiceInterfaceBuffer.h"

#if FSFW_CPP_OSTREAM_ENABLED == 1

#include <inttypes.h>

#include <cstring>

#include "fsfw/serviceinterface/serviceInterfaceDefintions.h"
#include "fsfw/timemanager/Clock.h"

#if defined(WIN32) && FSFW_COLORED_OUTPUT == 1
#include "Windows.h"
#endif

// to be implemented by bsp
extern "C" void printChar(const char*, bool errStream);

#ifndef UT699

ServiceInterfaceBuffer::ServiceInterfaceBuffer(std::string setMessage, bool addCrToPreamble,
                                               bool buffered, bool errStream, uint16_t port)
    : isActive(true),
      logMessage(setMessage),
      addCrToPreamble(addCrToPreamble),
      buffered(buffered),
      errStream(errStream) {
  if (buffered) {
    // Set pointers if the stream is buffered.
    setp(buf, buf + BUF_SIZE);
  }

#if FSFW_COLORED_OUTPUT == 1
  if (setMessage.find("DEBUG") != std::string::npos) {
    colorPrefix = sif::ANSI_COLOR_CYAN;
  } else if (setMessage.find("INFO") != std::string::npos) {
    colorPrefix = sif::ANSI_COLOR_GREEN;
  } else if (setMessage.find("WARNING") != std::string::npos) {
    colorPrefix = sif::ANSI_COLOR_MAGENTA;
  } else if (setMessage.find("ERROR") != std::string::npos) {
    colorPrefix = sif::ANSI_COLOR_RED;
  } else {
    colorPrefix = sif::ANSI_COLOR_RESET;
  }

#ifdef WIN32
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode = 0;
  GetConsoleMode(hOut, &dwMode);
  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hOut, dwMode);
#endif

#endif

  preamble.reserve(MAX_PREAMBLE_SIZE);
  preamble.resize(MAX_PREAMBLE_SIZE);
}

void ServiceInterfaceBuffer::putChars(char const* begin, char const* end) {
  if (not isActive) {
    return;
  }
  char array[BUF_SIZE];
  uint32_t length = end - begin;
  if (length > sizeof(array)) {
    length = sizeof(array);
  }
  memcpy(array, begin, length);

  for (; begin != end; begin++) {
    if (errStream) {
      printChar(begin, true);
    } else {
      printChar(begin, false);
    }
  }
}

#endif

int ServiceInterfaceBuffer::overflow(int c) {
  if (not buffered and this->isActive) {
    if (c != Traits::eof()) {
      if (errStream) {
        printChar(reinterpret_cast<const char*>(&c), true);
      } else {
        printChar(reinterpret_cast<const char*>(&c), false);
      }
    }
    return 0;
  }
  // Handle output
  putChars(pbase(), pptr());
  if (c != Traits::eof()) {
    char c2 = c;
    // Handle the one character that didn't fit to buffer
    putChars(&c2, &c2 + 1);
  }
  // This tells that buffer is empty again
  setp(buf, buf + BUF_SIZE - 1);
  // I'm not sure about this return value!
  return 0;
}

int ServiceInterfaceBuffer::sync(void) {
  if (not this->isActive and not buffered) {
    if (not buffered) {
      setp(buf, buf + BUF_SIZE - 1);
    }
    return 0;
  }
  if (not buffered) {
    return 0;
  }

  size_t preambleSize = 0;
  std::string* preamble = getPreamble(&preambleSize);
  // Write logMessage and time
  this->putChars(preamble->data(), preamble->data() + preambleSize);
  // Handle output
  this->putChars(pbase(), pptr());
  // This tells that buffer is empty again
  setp(buf, buf + BUF_SIZE - 1);
  return 0;
}

bool ServiceInterfaceBuffer::isBuffered() const { return buffered; }

std::string* ServiceInterfaceBuffer::getPreamble(size_t* preambleSize) {
  Clock::TimeOfDay_t loggerTime;
  Clock::getDateAndTime(&loggerTime);
  size_t currentSize = 0;
  char* parsePosition = &preamble[0];
  if (addCrToPreamble) {
    preamble[0] = '\r';
    currentSize += 1;
    parsePosition += 1;
  }

#if FSFW_COLORED_OUTPUT == 1
  currentSize += sprintf(parsePosition, "%s", colorPrefix.c_str());
  parsePosition += colorPrefix.size();
#endif

  int32_t charCount =
      sprintf(parsePosition, "%s%s | %02" SCNu32 ":%02" SCNu32 ":%02" SCNu32 ".%03" SCNu32 " | ",
              this->logMessage.c_str(), sif::ANSI_COLOR_RESET, loggerTime.hour, loggerTime.minute,
              loggerTime.second, loggerTime.usecond / 1000);
  if (charCount < 0) {
    printf("ServiceInterfaceBuffer: Failure parsing preamble\r\n");
    return &preamble;
  }
  if (charCount > MAX_PREAMBLE_SIZE) {
    printf(
        "ServiceInterfaceBuffer: Char count too large for maximum "
        "preamble size");
    return &preamble;
  }
  currentSize += charCount;
  if (preambleSize != nullptr) {
    *preambleSize = currentSize;
  }
  return &preamble;
}

bool ServiceInterfaceBuffer::crAdditionEnabled() const { return addCrToPreamble; }

#if FSFW_COLORED_OUTPUT == 1
void ServiceInterfaceBuffer::setAsciiColorPrefix(std::string colorPrefix) {
  this->colorPrefix = colorPrefix;
}
#endif

#ifdef UT699
#include "../osal/rtems/Interrupt.h"

ServiceInterfaceBuffer::ServiceInterfaceBuffer(std::string set_message, uint16_t port) {
  this->log_message = set_message;
  this->isActive = true;
  setp(buf, buf + BUF_SIZE);
}

void ServiceInterfaceBuffer::putChars(char const* begin, char const* end) {
  char array[BUF_SIZE];
  uint32_t length = end - begin;
  if (length > sizeof(array)) {
    length = sizeof(array);
  }
  memcpy(array, begin, length);

  if (!Interrupt::isInterruptInProgress()) {
    std::cout << array;
  } else {
    // Uncomment the following line if you need ISR debug output.
    //		printk(array);
  }
}
#endif  // UT699

#ifdef ML505
#include <bsp_flp/network/networkconfig.h>
ServiceInterfaceBuffer::ServiceInterfaceBuffer(std::string set_message, uint16_t port)
    : isActive(true),
      log_message(set_message),
      udpSocket(0),
      remoteAddressLength(sizeof(remoteAddress)) {
  setp(buf, buf + BUF_SIZE);
  memset((uint8_t*)&remoteAddress, 0, sizeof(remoteAddress));
  remoteAddress.sin_family = AF_INET;
  remoteAddress.sin_port = htons(port);
  remoteAddress.sin_addr.s_addr = htonl(inet_addr("192.168.250.100"));
}

void ServiceInterfaceBuffer::putChars(char const* begin, char const* end) {
  char array[BUF_SIZE];
  uint32_t length = end - begin;
  if (length > sizeof(array)) {
    length = sizeof(array);
  }
  memcpy(array, begin, length);

  if (udpSocket <= 0) {
    initSocket();
  }

  if (udpSocket > 0) {
    sendto(udpSocket, array, length, 0, (sockaddr*)&remoteAddress, sizeof(remoteAddress));
  }
}

void ServiceInterfaceBuffer::initSocket() {
  sockaddr_in address;
  memset((uint8_t*)&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(0);
  address.sin_addr.s_addr = htonl(INADDR_ANY);

  udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
  if (socket < 0) {
    printf("Error opening socket!\n");
    return;
  }
  timeval timeout = {0, 20};
  if (setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
    printf("Error setting SO_RCVTIMEO socket options!\n");
    return;
  }
  if (setsockopt(udpSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
    printf("Error setting SO_SNDTIMEO socket options!\n");
    return;
  }
  if (bind(udpSocket, (sockaddr*)&address, sizeof(address)) < 0) {
    printf("Error binding socket!\n");
  }
}

#endif  // ML505

#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
