#include "fsfw/osal/common/tcpipHelpers.h"

#include <errno.h>

#include <string>

#include "fsfw/serviceinterface/ServiceInterface.h"
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

#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "tcpip::handleError: " << protocolString << " | " << errorSrcString << " | "
               << infoString << std::endl;
#else
  sif::printWarning("tcpip::handleError: %s | %s | %s\n", protocolString.c_str(),
                    errorSrcString.c_str(), infoString.c_str());
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */

  if (sleepDuration > 0) {
    TaskFactory::instance()->delayTask(sleepDuration);
  }
}
