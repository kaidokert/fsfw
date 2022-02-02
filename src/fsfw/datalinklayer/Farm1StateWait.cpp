#include "fsfw/datalinklayer/Farm1StateWait.h"

#include "fsfw/datalinklayer/ClcwIF.h"
#include "fsfw/datalinklayer/TcTransferFrame.h"
#include "fsfw/datalinklayer/VirtualChannelReception.h"

Farm1StateWait::Farm1StateWait(VirtualChannelReception* setMyVC) : myVC(setMyVC) {}

ReturnValue_t Farm1StateWait::handleADFrame(TcTransferFrame* frame, ClcwIF* clcw) {
  int8_t diff = frame->getSequenceNumber() - myVC->vR;
  if (diff < -myVC->negativeWindow || diff >= myVC->positiveWindow) {
    clcw->setLockoutFlag(true);
    myVC->currentState = &(myVC->lockoutState);
  }
  return FARM_IN_WAIT;
}

ReturnValue_t Farm1StateWait::handleBCUnlockCommand(ClcwIF* clcw) {
  myVC->farmBCounter++;
  clcw->setRetransmitFlag(false);
  clcw->setWaitFlag(false);
  myVC->currentState = &(myVC->openState);
  return BC_IS_UNLOCK_COMMAND;
}

ReturnValue_t Farm1StateWait::handleBCSetVrCommand(ClcwIF* clcw, uint8_t vr) {
  myVC->farmBCounter++;
  clcw->setWaitFlag(false);
  clcw->setRetransmitFlag(false);
  myVC->vR = vr;
  myVC->currentState = &(myVC->openState);
  return BC_IS_SET_VR_COMMAND;
}
