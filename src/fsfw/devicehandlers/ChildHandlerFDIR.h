#ifndef FSFW_DEVICEHANDLERS_CHILDHANDLERFDIR_H_
#define FSFW_DEVICEHANDLERS_CHILDHANDLERFDIR_H_

#include "DeviceHandlerFailureIsolation.h"

/**
 * Very simple extension to normal FDIR.
 * Does not have a default fault tree parent and
 * allows to make the recovery count settable to 0.
 */
class ChildHandlerFDIR : public DeviceHandlerFailureIsolation {
 public:
  ChildHandlerFDIR(object_id_t owner, object_id_t faultTreeParent = NO_FAULT_TREE_PARENT,
                   uint32_t recoveryCount = 0);
  virtual ~ChildHandlerFDIR();

 protected:
  static const object_id_t NO_FAULT_TREE_PARENT = 0;
};

#endif /* FRAMEWORK_DEVICEHANDLERS_CHILDHANDLERFDIR_H_ */
