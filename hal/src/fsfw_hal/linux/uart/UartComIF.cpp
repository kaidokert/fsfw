#include "UartComIF.h"

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <cstring>

#include "fsfw/FSFW.h"
#include "fsfw/serviceinterface.h"
#include "fsfw_hal/linux/utility.h"

UartComIF::UartComIF(object_id_t objectId) : SystemObject(objectId) {}

UartComIF::~UartComIF() {}

ReturnValue_t UartComIF::initializeInterface(CookieIF* cookie) {
  std::string deviceFile;
  UartDeviceMapIter uartDeviceMapIter;

  if (cookie == nullptr) {
    return NULLPOINTER;
  }

  UartCookie* uartCookie = dynamic_cast<UartCookie*>(cookie);
  if (uartCookie == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "UartComIF::initializeInterface: Invalid UART Cookie!" << std::endl;
#endif
    return NULLPOINTER;
  }

  deviceFile = uartCookie->getDeviceFile();

  uartDeviceMapIter = uartDeviceMap.find(deviceFile);
  if (uartDeviceMapIter == uartDeviceMap.end()) {
    int fileDescriptor = configureUartPort(uartCookie);
    if (fileDescriptor < 0) {
      return RETURN_FAILED;
    }
    size_t maxReplyLen = uartCookie->getMaxReplyLen();
    UartElements uartElements = {fileDescriptor, std::vector<uint8_t>(maxReplyLen), 0};
    auto status = uartDeviceMap.emplace(deviceFile, uartElements);
    if (status.second == false) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "UartComIF::initializeInterface: Failed to insert device " << deviceFile
                   << "to UART device map" << std::endl;
#endif
      return RETURN_FAILED;
    }
  } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "UartComIF::initializeInterface: UART device " << deviceFile
                 << " already in use" << std::endl;
#endif
    return RETURN_FAILED;
  }

  return RETURN_OK;
}

int UartComIF::configureUartPort(UartCookie* uartCookie) {
  struct termios options = {};

  std::string deviceFile = uartCookie->getDeviceFile();
  int flags = O_RDWR;
  if (uartCookie->getUartMode() == UartModes::CANONICAL) {
    // In non-canonical mode, don't specify O_NONBLOCK because these properties will be
    // controlled by the VTIME and VMIN parameters and O_NONBLOCK would override this
    flags |= O_NONBLOCK;
  }
  int fd = open(deviceFile.c_str(), flags);

  if (fd < 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "UartComIF::configureUartPort: Failed to open uart " << deviceFile
                 << "with error code " << errno << strerror(errno) << std::endl;
#endif
    return fd;
  }

  /* Read in existing settings */
  if (tcgetattr(fd, &options) != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "UartComIF::configureUartPort: Error " << errno
                 << "from tcgetattr: " << strerror(errno) << std::endl;
#endif
    return fd;
  }

  setParityOptions(&options, uartCookie);
  setStopBitOptions(&options, uartCookie);
  setDatasizeOptions(&options, uartCookie);
  setFixedOptions(&options);
  setUartMode(&options, *uartCookie);
  if (uartCookie->getInputShouldBeFlushed()) {
    tcflush(fd, TCIFLUSH);
  }

  /* Sets uart to non-blocking mode. Read returns immediately when there are no data available */
  options.c_cc[VTIME] = 0;
  options.c_cc[VMIN] = 0;

  configureBaudrate(&options, uartCookie);

  /* Save option settings */
  if (tcsetattr(fd, TCSANOW, &options) != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "UartComIF::configureUartPort: Failed to set options with error " << errno
                 << ": " << strerror(errno);
#endif
    return fd;
  }
  return fd;
}

void UartComIF::setParityOptions(struct termios* options, UartCookie* uartCookie) {
  /* Clear parity bit */
  options->c_cflag &= ~PARENB;
  switch (uartCookie->getParity()) {
    case Parity::EVEN:
      options->c_cflag |= PARENB;
      options->c_cflag &= ~PARODD;
      break;
    case Parity::ODD:
      options->c_cflag |= PARENB;
      options->c_cflag |= PARODD;
      break;
    default:
      break;
  }
}

void UartComIF::setStopBitOptions(struct termios* options, UartCookie* uartCookie) {
  /* Clear stop field. Sets stop bit to one bit */
  options->c_cflag &= ~CSTOPB;
  switch (uartCookie->getStopBits()) {
    case StopBits::TWO_STOP_BITS:
      options->c_cflag |= CSTOPB;
      break;
    default:
      break;
  }
}

void UartComIF::setDatasizeOptions(struct termios* options, UartCookie* uartCookie) {
  /* Clear size bits */
  options->c_cflag &= ~CSIZE;
  switch (uartCookie->getBitsPerWord()) {
    case 5:
      options->c_cflag |= CS5;
      break;
    case 6:
      options->c_cflag |= CS6;
      break;
    case 7:
      options->c_cflag |= CS7;
      break;
    case 8:
      options->c_cflag |= CS8;
      break;
    default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "UartComIF::setDatasizeOptions: Invalid size specified" << std::endl;
#endif
      break;
  }
}

void UartComIF::setFixedOptions(struct termios* options) {
  /* Disable RTS/CTS hardware flow control */
  options->c_cflag &= ~CRTSCTS;
  /* Turn on READ & ignore ctrl lines (CLOCAL = 1) */
  options->c_cflag |= CREAD | CLOCAL;
  /* Disable echo */
  options->c_lflag &= ~ECHO;
  /* Disable erasure */
  options->c_lflag &= ~ECHOE;
  /* Disable new-line echo */
  options->c_lflag &= ~ECHONL;
  /* Disable interpretation of INTR, QUIT and SUSP */
  options->c_lflag &= ~ISIG;
  /* Turn off s/w flow ctrl */
  options->c_iflag &= ~(IXON | IXOFF | IXANY);
  /* Disable any special handling of received bytes */
  options->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
  /* Prevent special interpretation of output bytes (e.g. newline chars) */
  options->c_oflag &= ~OPOST;
  /* Prevent conversion of newline to carriage return/line feed */
  options->c_oflag &= ~ONLCR;
}

void UartComIF::configureBaudrate(struct termios* options, UartCookie* uartCookie) {
  switch (uartCookie->getBaudrate()) {
    case 50:
      cfsetispeed(options, B50);
      cfsetospeed(options, B50);
      break;
    case 75:
      cfsetispeed(options, B75);
      cfsetospeed(options, B75);
      break;
    case 110:
      cfsetispeed(options, B110);
      cfsetospeed(options, B110);
      break;
    case 134:
      cfsetispeed(options, B134);
      cfsetospeed(options, B134);
      break;
    case 150:
      cfsetispeed(options, B150);
      cfsetospeed(options, B150);
      break;
    case 200:
      cfsetispeed(options, B200);
      cfsetospeed(options, B200);
      break;
    case 300:
      cfsetispeed(options, B300);
      cfsetospeed(options, B300);
      break;
    case 600:
      cfsetispeed(options, B600);
      cfsetospeed(options, B600);
      break;
    case 1200:
      cfsetispeed(options, B1200);
      cfsetospeed(options, B1200);
      break;
    case 1800:
      cfsetispeed(options, B1800);
      cfsetospeed(options, B1800);
      break;
    case 2400:
      cfsetispeed(options, B2400);
      cfsetospeed(options, B2400);
      break;
    case 4800:
      cfsetispeed(options, B4800);
      cfsetospeed(options, B4800);
      break;
    case 9600:
      cfsetispeed(options, B9600);
      cfsetospeed(options, B9600);
      break;
    case 19200:
      cfsetispeed(options, B19200);
      cfsetospeed(options, B19200);
      break;
    case 38400:
      cfsetispeed(options, B38400);
      cfsetospeed(options, B38400);
      break;
    case 57600:
      cfsetispeed(options, B57600);
      cfsetospeed(options, B57600);
      break;
    case 115200:
      cfsetispeed(options, B115200);
      cfsetospeed(options, B115200);
      break;
    case 230400:
      cfsetispeed(options, B230400);
      cfsetospeed(options, B230400);
      break;
    case 460800:
      cfsetispeed(options, B460800);
      cfsetospeed(options, B460800);
      break;
    default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "UartComIF::configureBaudrate: Baudrate not supported" << std::endl;
#endif
      break;
  }
}

ReturnValue_t UartComIF::sendMessage(CookieIF* cookie, const uint8_t* sendData, size_t sendLen) {
  int fd = 0;
  std::string deviceFile;
  UartDeviceMapIter uartDeviceMapIter;

  if (sendLen == 0) {
    return RETURN_OK;
  }

  if (sendData == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "UartComIF::sendMessage: Send data is nullptr" << std::endl;
#endif
    return RETURN_FAILED;
  }

  UartCookie* uartCookie = dynamic_cast<UartCookie*>(cookie);
  if (uartCookie == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "UartComIF::sendMessasge: Invalid UART Cookie!" << std::endl;
#endif
    return NULLPOINTER;
  }

  deviceFile = uartCookie->getDeviceFile();
  uartDeviceMapIter = uartDeviceMap.find(deviceFile);
  if (uartDeviceMapIter == uartDeviceMap.end()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "UartComIF::sendMessage: Device file " << deviceFile << "not in UART map"
               << std::endl;
#endif
    return RETURN_FAILED;
  }

  fd = uartDeviceMapIter->second.fileDescriptor;

  if (write(fd, sendData, sendLen) != static_cast<int>(sendLen)) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "UartComIF::sendMessage: Failed to send data with error code " << errno
               << ": Error description: " << strerror(errno) << std::endl;
#endif
    return RETURN_FAILED;
  }

  return RETURN_OK;
}

ReturnValue_t UartComIF::getSendSuccess(CookieIF* cookie) { return RETURN_OK; }

ReturnValue_t UartComIF::requestReceiveMessage(CookieIF* cookie, size_t requestLen) {
  std::string deviceFile;
  UartDeviceMapIter uartDeviceMapIter;

  UartCookie* uartCookie = dynamic_cast<UartCookie*>(cookie);
  if (uartCookie == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "UartComIF::requestReceiveMessage: Invalid Uart Cookie!" << std::endl;
#endif
    return NULLPOINTER;
  }

  UartModes uartMode = uartCookie->getUartMode();
  deviceFile = uartCookie->getDeviceFile();
  uartDeviceMapIter = uartDeviceMap.find(deviceFile);

  if (uartMode == UartModes::NON_CANONICAL and requestLen == 0) {
    return RETURN_OK;
  }

  if (uartDeviceMapIter == uartDeviceMap.end()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "UartComIF::requestReceiveMessage: Device file " << deviceFile
               << " not in uart map" << std::endl;
#endif
    return RETURN_FAILED;
  }

  if (uartMode == UartModes::CANONICAL) {
    return handleCanonicalRead(*uartCookie, uartDeviceMapIter, requestLen);
  } else if (uartMode == UartModes::NON_CANONICAL) {
    return handleNoncanonicalRead(*uartCookie, uartDeviceMapIter, requestLen);
  } else {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
}

ReturnValue_t UartComIF::handleCanonicalRead(UartCookie& uartCookie, UartDeviceMapIter& iter,
                                             size_t requestLen) {
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  uint8_t maxReadCycles = uartCookie.getReadCycles();
  uint8_t currentReadCycles = 0;
  int bytesRead = 0;
  size_t currentBytesRead = 0;
  size_t maxReplySize = uartCookie.getMaxReplyLen();
  int fd = iter->second.fileDescriptor;
  auto bufferPtr = iter->second.replyBuffer.data();
  iter->second.replyLen = 0;
  do {
    size_t allowedReadSize = 0;
    if (currentBytesRead >= maxReplySize) {
      // Overflow risk. Emit warning, trigger event and break. If this happens,
      // the reception buffer is not large enough or data is not polled often enough.
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "UartComIF::requestReceiveMessage: Next read would cause overflow!"
                   << std::endl;
#else
      sif::printWarning(
          "UartComIF::requestReceiveMessage: "
          "Next read would cause overflow!");
#endif
#endif
      result = UART_RX_BUFFER_TOO_SMALL;
      break;
    } else {
      allowedReadSize = maxReplySize - currentBytesRead;
    }

    bytesRead = read(fd, bufferPtr, allowedReadSize);
    if (bytesRead < 0) {
      // EAGAIN: No data available in non-blocking mode
      if (errno != EAGAIN) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "UartComIF::handleCanonicalRead: read failed with code" << errno << ": "
                     << strerror(errno) << std::endl;
#else
        sif::printWarning("UartComIF::handleCanonicalRead: read failed with code %d: %s\n", errno,
                          strerror(errno));
#endif
#endif
        return RETURN_FAILED;
      }

    } else if (bytesRead > 0) {
      iter->second.replyLen += bytesRead;
      bufferPtr += bytesRead;
      currentBytesRead += bytesRead;
    }
    currentReadCycles++;
  } while (bytesRead > 0 and currentReadCycles < maxReadCycles);
  return result;
}

ReturnValue_t UartComIF::handleNoncanonicalRead(UartCookie& uartCookie, UartDeviceMapIter& iter,
                                                size_t requestLen) {
  int fd = iter->second.fileDescriptor;
  auto bufferPtr = iter->second.replyBuffer.data();
  // Size check to prevent buffer overflow
  if (requestLen > uartCookie.getMaxReplyLen()) {
#if OBSW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "UartComIF::requestReceiveMessage: Next read would cause overflow!"
                 << std::endl;
#else
    sif::printWarning(
        "UartComIF::requestReceiveMessage: "
        "Next read would cause overflow!");
#endif
#endif
    return UART_RX_BUFFER_TOO_SMALL;
  }
  int bytesRead = read(fd, bufferPtr, requestLen);
  if (bytesRead < 0) {
    return RETURN_FAILED;
  } else if (bytesRead != static_cast<int>(requestLen)) {
    if (uartCookie.isReplySizeFixed()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "UartComIF::requestReceiveMessage: Only read " << bytesRead << " of "
                   << requestLen << " bytes" << std::endl;
#endif
      return RETURN_FAILED;
    }
  }
  iter->second.replyLen = bytesRead;
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t UartComIF::readReceivedMessage(CookieIF* cookie, uint8_t** buffer, size_t* size) {
  std::string deviceFile;
  UartDeviceMapIter uartDeviceMapIter;

  UartCookie* uartCookie = dynamic_cast<UartCookie*>(cookie);
  if (uartCookie == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "UartComIF::readReceivedMessage: Invalid uart cookie!" << std::endl;
#endif
    return NULLPOINTER;
  }

  deviceFile = uartCookie->getDeviceFile();
  uartDeviceMapIter = uartDeviceMap.find(deviceFile);
  if (uartDeviceMapIter == uartDeviceMap.end()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "UartComIF::readReceivedMessage: Device file " << deviceFile << " not in uart map"
               << std::endl;
#endif
    return RETURN_FAILED;
  }

  *buffer = uartDeviceMapIter->second.replyBuffer.data();
  *size = uartDeviceMapIter->second.replyLen;

  /* Length is reset to 0 to prevent reading the same data twice */
  uartDeviceMapIter->second.replyLen = 0;

  return RETURN_OK;
}

ReturnValue_t UartComIF::flushUartRxBuffer(CookieIF* cookie) {
  std::string deviceFile;
  UartDeviceMapIter uartDeviceMapIter;
  UartCookie* uartCookie = dynamic_cast<UartCookie*>(cookie);
  if (uartCookie == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "UartComIF::flushUartRxBuffer: Invalid uart cookie!" << std::endl;
#endif
    return NULLPOINTER;
  }
  deviceFile = uartCookie->getDeviceFile();
  uartDeviceMapIter = uartDeviceMap.find(deviceFile);
  if (uartDeviceMapIter != uartDeviceMap.end()) {
    int fd = uartDeviceMapIter->second.fileDescriptor;
    tcflush(fd, TCIFLUSH);
    return RETURN_OK;
  }
  return RETURN_FAILED;
}

ReturnValue_t UartComIF::flushUartTxBuffer(CookieIF* cookie) {
  std::string deviceFile;
  UartDeviceMapIter uartDeviceMapIter;
  UartCookie* uartCookie = dynamic_cast<UartCookie*>(cookie);
  if (uartCookie == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "UartComIF::flushUartTxBuffer: Invalid uart cookie!" << std::endl;
#endif
    return NULLPOINTER;
  }
  deviceFile = uartCookie->getDeviceFile();
  uartDeviceMapIter = uartDeviceMap.find(deviceFile);
  if (uartDeviceMapIter != uartDeviceMap.end()) {
    int fd = uartDeviceMapIter->second.fileDescriptor;
    tcflush(fd, TCOFLUSH);
    return RETURN_OK;
  }
  return RETURN_FAILED;
}

ReturnValue_t UartComIF::flushUartTxAndRxBuf(CookieIF* cookie) {
  std::string deviceFile;
  UartDeviceMapIter uartDeviceMapIter;
  UartCookie* uartCookie = dynamic_cast<UartCookie*>(cookie);
  if (uartCookie == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "UartComIF::flushUartTxAndRxBuf: Invalid uart cookie!" << std::endl;
#endif
    return NULLPOINTER;
  }
  deviceFile = uartCookie->getDeviceFile();
  uartDeviceMapIter = uartDeviceMap.find(deviceFile);
  if (uartDeviceMapIter != uartDeviceMap.end()) {
    int fd = uartDeviceMapIter->second.fileDescriptor;
    tcflush(fd, TCIOFLUSH);
    return RETURN_OK;
  }
  return RETURN_FAILED;
}

void UartComIF::setUartMode(struct termios* options, UartCookie& uartCookie) {
  UartModes uartMode = uartCookie.getUartMode();
  if (uartMode == UartModes::NON_CANONICAL) {
    /* Disable canonical mode */
    options->c_lflag &= ~ICANON;
  } else if (uartMode == UartModes::CANONICAL) {
    options->c_lflag |= ICANON;
  }
}
