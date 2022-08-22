#ifndef FSFW_OSAL_HOST_TASKHELPERS_H_
#define FSFW_OSAL_HOST_TASKHELPERS_H_

#include <fsfw/returnvalues/returnvalue.h>

#include <thread>

namespace tasks {

ReturnValue_t insertTaskName(std::thread::id threadId, const std::string& taskName);
std::string getTaskName(std::thread::id threadId);

}  // namespace tasks

#endif /* FSFW_OSAL_HOST_TASKHELPERS_H_ */
