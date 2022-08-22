#ifndef POLLINGSEQUENCEFACTORY_H_
#define POLLINGSEQUENCEFACTORY_H_

#include <fsfw/returnvalues/returnvalue.h>

class FixedTimeslotTaskIF;

/**
 * All device handlers are scheduled by adding them into  Polling Sequence Tables (PST)
 * to satisfy stricter timing requirements of device communication,
 * A device handler has four different communication steps:
 *   1. DeviceHandlerIF::SEND_WRITE -> Send write via interface
 *   2. DeviceHandlerIF::GET_WRITE -> Get confirmation for write
 *   3. DeviceHandlerIF::SEND_READ -> Send read request
 *   4. DeviceHandlerIF::GET_READ -> Read from interface
 * The PST specifies precisely when the respective ComIF functions are called
 * during the communication cycle time.
 * The task is created using the FixedTimeslotTaskIF,
 * which utilises the underlying Operating System Abstraction Layer (OSAL)
 *
 * @param thisSequence FixedTimeslotTaskIF * object is passed inside the Factory class when creating
 * the PST
 * @return
 */
namespace pst {

/* Default PST */
ReturnValue_t pollingSequenceInitDefault(FixedTimeslotTaskIF *thisSequence);

}  // namespace pst

#endif /* POLLINGSEQUENCEINIT_H_ */
