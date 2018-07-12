#ifndef FRAMEWORK_OSAL_ENDINESS_H_
#define FRAMEWORK_OSAL_ENDINESS_H_

//We have to define BYTE_ORDER for that specific OS/Hardware so this must be done somewhere
#ifndef API
#error Please specify Operating System API. Supported: API=RTEMS_API
#elif API == RTEMS_API
#include "rtems/RtemsBasic.h"
#endif



#endif /* FRAMEWORK_OSAL_ENDINESS_H_ */
