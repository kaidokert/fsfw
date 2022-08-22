#include "fsfw/modes/ModeHelper.h"

#include "fsfw/ipc/MessageQueueSenderIF.h"
#include "fsfw/modes/HasModesIF.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

ModeHelper::ModeHelper(HasModesIF* owner)
    : commandedMode(HasModesIF::MODE_OFF),
      commandedSubmode(HasModesIF::SUBMODE_NONE),
      owner(owner),
      forced(false) {}

ModeHelper::~ModeHelper() {}

ReturnValue_t ModeHelper::handleModeCommand(CommandMessage* command) {
  CommandMessage reply;
  Mode_t mode;
  Submode_t submode;
  switch (command->getCommand()) {
    case ModeMessage::CMD_MODE_COMMAND_FORCED:
      forced = true;
      /* NO BREAK falls through*/
    case ModeMessage::CMD_MODE_COMMAND: {
      mode = ModeMessage::getMode(command);
      submode = ModeMessage::getSubmode(command);
      uint32_t timeout;
      ReturnValue_t result = owner->checkModeCommand(mode, submode, &timeout);
      if (result != returnvalue::OK) {
        ModeMessage::setCantReachMode(&reply, result);
        MessageQueueSenderIF::sendMessage(command->getSender(), &reply, owner->getCommandQueue());
        break;
      }
      // Free to start transition
      theOneWhoCommandedAMode = command->getSender();
      commandedMode = mode;
      commandedSubmode = submode;

      if ((parentQueueId != MessageQueueIF::NO_QUEUE) &&
          (theOneWhoCommandedAMode != parentQueueId)) {
        owner->setToExternalControl();
      }

      countdown.setTimeout(timeout);
      owner->startTransition(mode, submode);
    } break;
    case ModeMessage::CMD_MODE_READ: {
      owner->getMode(&mode, &submode);
      ModeMessage::setModeMessage(&reply, ModeMessage::REPLY_MODE_REPLY, mode, submode);
      MessageQueueSenderIF::sendMessage(command->getSender(), &reply, owner->getCommandQueue());
    } break;
    case ModeMessage::CMD_MODE_ANNOUNCE:
      owner->announceMode(false);
      break;
    case ModeMessage::CMD_MODE_ANNOUNCE_RECURSIVELY:
      owner->announceMode(true);
      break;
    default:
      return returnvalue::FAILED;
  }
  return returnvalue::OK;
}

ReturnValue_t ModeHelper::initialize(MessageQueueId_t parentQueueId) {
  setParentQueue(parentQueueId);
  return initialize();
}

void ModeHelper::modeChanged(Mode_t ownerMode, Submode_t ownerSubmode) {
  forced = false;
  sendModeReplyMessage(ownerMode, ownerSubmode);
  sendModeInfoMessage(ownerMode, ownerSubmode);
  theOneWhoCommandedAMode = MessageQueueIF::NO_QUEUE;
}

void ModeHelper::sendModeReplyMessage(Mode_t ownerMode, Submode_t ownerSubmode) {
  CommandMessage reply;
  if (theOneWhoCommandedAMode != MessageQueueIF::NO_QUEUE) {
    if (ownerMode != commandedMode or ownerSubmode != commandedSubmode) {
      ModeMessage::setModeMessage(&reply, ModeMessage::REPLY_WRONG_MODE_REPLY, ownerMode,
                                  ownerSubmode);
    } else {
      ModeMessage::setModeMessage(&reply, ModeMessage::REPLY_MODE_REPLY, ownerMode, ownerSubmode);
    }
    MessageQueueSenderIF::sendMessage(theOneWhoCommandedAMode, &reply, owner->getCommandQueue());
  }
}

void ModeHelper::sendModeInfoMessage(Mode_t ownerMode, Submode_t ownerSubmode) {
  CommandMessage reply;
  if (theOneWhoCommandedAMode != parentQueueId and parentQueueId != MessageQueueIF::NO_QUEUE) {
    ModeMessage::setModeMessage(&reply, ModeMessage::REPLY_MODE_INFO, ownerMode, ownerSubmode);
    MessageQueueSenderIF::sendMessage(parentQueueId, &reply, owner->getCommandQueue());
  }
}

void ModeHelper::startTimer(uint32_t timeoutMs) { countdown.setTimeout(timeoutMs); }

void ModeHelper::setParentQueue(MessageQueueId_t parentQueueId) {
  this->parentQueueId = parentQueueId;
}

ReturnValue_t ModeHelper::initialize(void) { return returnvalue::OK; }

bool ModeHelper::isTimedOut() { return countdown.hasTimedOut(); }

bool ModeHelper::isForced() { return forced; }

void ModeHelper::setForced(bool forced) { this->forced = forced; }
