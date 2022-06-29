#pragma once

#include "Action.h"

template <class owner, class action, class ActionEnum>
class TemplateAction : public Action {
 public:
#ifdef FSFW_INTROSPECTION
  TemplateAction(owner *myOwner, ActionEnum id) : Action(), myOwner(myOwner) {
    Action::setEnum(&id);
    myOwner->getActionHelper()->registerAction(this);
  }
#else
  TemplateAction(owner *myOwner, ActionEnum id) : Action((uint32_t) id), myOwner(myOwner) {
    myOwner->getActionHelper()->registerAction(this);
  }
#endif

  ReturnValue_t handle() override { return myOwner->handleAction(dynamic_cast<action *>(this)); }

 private:
  owner *myOwner;
};