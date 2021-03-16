#ifndef FSFW_OSAL_WINDOWS_TCPIPHELPERS_H_
#define FSFW_OSAL_WINDOWS_TCPIPHELPERS_H_

#include "../../timemanager/clockDefinitions.h"
#include "../common/tcpipCommon.h"

namespace tcpip {

void handleError(Protocol protocol, ErrorSources errorSrc, dur_millis_t sleepDuration = 0);

}



#endif /* FSFW_OSAL_WINDOWS_TCPIPHELPERS_H_ */
