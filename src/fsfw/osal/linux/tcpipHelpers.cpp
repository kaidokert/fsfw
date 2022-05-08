#include "fsfw/osal/common/tcpipHelpers.h"

#include <cerrno>
#include <string>

#include "fsfw/serviceinterface.h"
#include "fsfw/tasks/TaskFactory.h"

void tcpip::handleError(Protocol protocol, ErrorSources errorSrc, dur_millis_t sleepDuration) {
  int errCode = errno;
  std::string protocolString;
  std::string errorSrcString;
  determineErrorStrings(protocol, errorSrc, protocolString, errorSrcString);
  std::string infoString;
  switch (errCode) {
    case (EACCES): {
      infoString = "EACCES";
      break;
    }
    case (EINVAL): {
      infoString = "EINVAL";
      break;
    }
    case (EAGAIN): {
      infoString = "EAGAIN";
      break;
    }
    case (EMFILE): {
      infoString = "EMFILE";
      break;
    }
    case (ENFILE): {
      infoString = "ENFILE";
      break;
    }
    case (EAFNOSUPPORT): {
      infoString = "EAFNOSUPPORT";
      break;
    }
    case (ENOBUFS): {
      infoString = "ENOBUFS";
      break;
    }
    case (ENOMEM): {
      infoString = "ENOMEM";
      break;
    }
    case (EPROTONOSUPPORT): {
      infoString = "EPROTONOSUPPORT";
      break;
    }
    case (EADDRINUSE): {
      infoString = "EADDRINUSE";
      break;
    }
    case (EBADF): {
      infoString = "EBADF";
      break;
    }
    case (ENOTSOCK): {
      infoString = "ENOTSOCK";
      break;
    }
    case (EADDRNOTAVAIL): {
      infoString = "EADDRNOTAVAIL";
      break;
    }
    case (EFAULT): {
      infoString = "EFAULT";
      break;
    }
    case (ELOOP): {
      infoString = "ELOOP";
      break;
    }
    case (ENAMETOOLONG): {
      infoString = "ENAMETOOLONG";
      break;
    }
    case (ENOENT): {
      infoString = "ENOENT";
      break;
    }
    case (ENOTDIR): {
      infoString = "ENOTDIR";
      break;
    }
    case (EROFS): {
      infoString = "EROFS";
      break;
    }

    default: {
      infoString = "Error code: " + std::to_string(errCode);
    }
  }

  FSFW_LOGWT("tcpip::handleError: {} | {} | {}\n", protocolString, errorSrcString, infoString);

  if (sleepDuration > 0) {
    TaskFactory::delayTask(sleepDuration);
  }
}
