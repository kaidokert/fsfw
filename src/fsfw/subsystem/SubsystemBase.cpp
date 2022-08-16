#include "fsfw/subsystem/SubsystemBase.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

SubsystemBase::SubsystemBase(object_id_t setObjectId, object_id_t parent, Mode_t initialMode,
                             uint16_t commandQueueDepth)
    : SystemObject(setObjectId),
      mode(initialMode),
      commandQueue(QueueFactory::instance()->createMessageQueue(commandQueueDepth,
                                                                CommandMessage::MAX_MESSAGE_SIZE)),
      healthHelper(this, setObjectId),
      modeHelper(this),
      parentId(parent) {}

SubsystemBase::~SubsystemBase() { QueueFactory::instance()->deleteMessageQueue(commandQueue); }

ReturnValue_t SubsystemBase::registerChild(object_id_t objectId) {
  ChildInfo info;

  HasModesIF* child = ObjectManager::instance()->get<HasModesIF>(objectId);
  // This is a rather ugly hack to have the changedHealth info for all
  // children available.
  HasHealthIF* healthChild = ObjectManager::instance()->get<HasHealthIF>(objectId);
  if (child == nullptr) {
    if (healthChild == nullptr) {
      return CHILD_DOESNT_HAVE_MODES;
    } else {
      info.commandQueue = healthChild->getCommandQueue();
      info.mode = MODE_OFF;
    }
  } else {
    info.commandQueue = child->getCommandQueue();
    info.mode = -1;  // intentional to force an initial command during system startup
  }

  info.submode = SUBMODE_NONE;
  info.healthChanged = false;

  auto resultPair = childrenMap.emplace(objectId, info);
  if (not resultPair.second) {
    return COULD_NOT_INSERT_CHILD;
  }
  return returnvalue::OK;
}

ReturnValue_t SubsystemBase::checkStateAgainstTable(HybridIterator<ModeListEntry> tableIter,
                                                    Submode_t targetSubmode) {
  std::map<object_id_t, ChildInfo>::iterator childIter;

  for (; tableIter.value != NULL; ++tableIter) {
    object_id_t object = tableIter.value->getObject();

    if ((childIter = childrenMap.find(object)) == childrenMap.end()) {
      return returnvalue::FAILED;
    }

    if (childIter->second.mode != tableIter.value->getMode()) {
      return returnvalue::FAILED;
    }

    Submode_t submodeToCheckAgainst = tableIter.value->getSubmode();
    if (tableIter.value->inheritSubmode()) {
      submodeToCheckAgainst = targetSubmode;
    }

    if (childIter->second.submode != submodeToCheckAgainst) {
      return returnvalue::FAILED;
    }
  }
  return returnvalue::OK;
}

void SubsystemBase::executeTable(HybridIterator<ModeListEntry> tableIter, Submode_t targetSubmode) {
  CommandMessage command;

  std::map<object_id_t, ChildInfo>::iterator iter;

  commandsOutstanding = 0;

  for (; tableIter.value != nullptr; ++tableIter) {
    object_id_t object = tableIter.value->getObject();
    if ((iter = childrenMap.find(object)) == childrenMap.end()) {
      // illegal table entry, should only happen due to misconfigured mode table
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::debug << std::hex << getObjectId() << ": invalid mode table entry" << std::endl;
#endif
      continue;
    }

    Submode_t submodeToCommand = tableIter.value->getSubmode();
    if (tableIter.value->inheritSubmode()) {
      submodeToCommand = targetSubmode;
    }

    if (healthHelper.healthTable->hasHealth(object)) {
      if (healthHelper.healthTable->isFaulty(object)) {
        ModeMessage::setModeMessage(&command, ModeMessage::CMD_MODE_COMMAND, HasModesIF::MODE_OFF,
                                    SUBMODE_NONE);
      } else {
        if (modeHelper.isForced()) {
          ModeMessage::setModeMessage(&command, ModeMessage::CMD_MODE_COMMAND_FORCED,
                                      tableIter.value->getMode(), submodeToCommand);
        } else {
          if (healthHelper.healthTable->isCommandable(object)) {
            ModeMessage::setModeMessage(&command, ModeMessage::CMD_MODE_COMMAND,
                                        tableIter.value->getMode(), submodeToCommand);
          } else {
            continue;
          }
        }
      }
    } else {
      ModeMessage::setModeMessage(&command, ModeMessage::CMD_MODE_COMMAND,
                                  tableIter.value->getMode(), submodeToCommand);
    }

    if ((iter->second.mode == ModeMessage::getMode(&command)) &&
        (iter->second.submode == ModeMessage::getSubmode(&command)) && !modeHelper.isForced()) {
      continue;  // don't send redundant mode commands (produces event spam), but still command if
                 // mode is forced to reach lower levels
    }
    ReturnValue_t result = commandQueue->sendMessage(iter->second.commandQueue, &command);
    if (result == returnvalue::OK) {
      ++commandsOutstanding;
    }
  }
}

ReturnValue_t SubsystemBase::updateChildMode(MessageQueueId_t queue, Mode_t mode,
                                             Submode_t submode) {
  std::map<object_id_t, ChildInfo>::iterator iter;

  for (iter = childrenMap.begin(); iter != childrenMap.end(); iter++) {
    if (iter->second.commandQueue == queue) {
      iter->second.mode = mode;
      iter->second.submode = submode;
      return returnvalue::OK;
    }
  }
  return CHILD_NOT_FOUND;
}

ReturnValue_t SubsystemBase::updateChildChangedHealth(MessageQueueId_t queue, bool changedHealth) {
  for (auto iter = childrenMap.begin(); iter != childrenMap.end(); iter++) {
    if (iter->second.commandQueue == queue) {
      iter->second.healthChanged = changedHealth;
      return returnvalue::OK;
    }
  }
  return CHILD_NOT_FOUND;
}

MessageQueueId_t SubsystemBase::getCommandQueue() const { return commandQueue->getId(); }

ReturnValue_t SubsystemBase::initialize() {
  MessageQueueId_t parentQueue = MessageQueueIF::NO_QUEUE;
  ReturnValue_t result = SystemObject::initialize();

  if (result != returnvalue::OK) {
    return result;
  }

  if (parentId != objects::NO_OBJECT) {
    SubsystemBase* parent = ObjectManager::instance()->get<SubsystemBase>(parentId);
    if (parent == nullptr) {
      return returnvalue::FAILED;
    }
    parentQueue = parent->getCommandQueue();

    parent->registerChild(getObjectId());
  }

  result = healthHelper.initialize(parentQueue);

  if (result != returnvalue::OK) {
    return result;
  }

  result = modeHelper.initialize(parentQueue);

  if (result != returnvalue::OK) {
    return result;
  }

  return returnvalue::OK;
}

ReturnValue_t SubsystemBase::performOperation(uint8_t opCode) {
  childrenChangedMode = false;

  checkCommandQueue();

  performChildOperation();

  return returnvalue::OK;
}

ReturnValue_t SubsystemBase::handleModeReply(CommandMessage* message) {
  switch (message->getCommand()) {
    case ModeMessage::REPLY_MODE_INFO:
      updateChildMode(message->getSender(), ModeMessage::getMode(message),
                      ModeMessage::getSubmode(message));
      childrenChangedMode = true;
      return returnvalue::OK;
    case ModeMessage::REPLY_MODE_REPLY:
    case ModeMessage::REPLY_WRONG_MODE_REPLY:
      updateChildMode(message->getSender(), ModeMessage::getMode(message),
                      ModeMessage::getSubmode(message));
      childrenChangedMode = true;
      commandsOutstanding--;
      return returnvalue::OK;
    case ModeMessage::REPLY_CANT_REACH_MODE:
      commandsOutstanding--;
      {
        for (auto iter = childrenMap.begin(); iter != childrenMap.end(); iter++) {
          if (iter->second.commandQueue == message->getSender()) {
            triggerEvent(MODE_CMD_REJECTED, iter->first, message->getParameter());
          }
        }
      }
      return returnvalue::OK;
      //	case ModeMessage::CMD_MODE_COMMAND:
      //		handleCommandedMode(message);
      //		return returnvalue::OK;
      //	case ModeMessage::CMD_MODE_ANNOUNCE:
      //		triggerEvent(MODE_INFO, mode, submode);
      //		return returnvalue::OK;
      //	case ModeMessage::CMD_MODE_ANNOUNCE_RECURSIVELY:
      //		triggerEvent(MODE_INFO, mode, submode);
      //		commandAllChildren(message);
      //		return returnvalue::OK;
    default:
      return returnvalue::FAILED;
  }
}

ReturnValue_t SubsystemBase::checkTable(HybridIterator<ModeListEntry> tableIter) {
  for (; tableIter.value != NULL; ++tableIter) {
    if (childrenMap.find(tableIter.value->getObject()) == childrenMap.end()) {
      return TABLE_CONTAINS_INVALID_OBJECT_ID;
    }
  }
  return returnvalue::OK;
}

void SubsystemBase::replyToCommand(CommandMessage* message) { commandQueue->reply(message); }

void SubsystemBase::setMode(Mode_t newMode, Submode_t newSubmode) {
  modeHelper.modeChanged(newMode, newSubmode);
  mode = newMode;
  submode = newSubmode;
  modeChanged();
  announceMode(false);
}

void SubsystemBase::setMode(Mode_t newMode) { setMode(newMode, submode); }

void SubsystemBase::commandAllChildren(CommandMessage* message) {
  std::map<object_id_t, ChildInfo>::iterator iter;
  for (iter = childrenMap.begin(); iter != childrenMap.end(); ++iter) {
    commandQueue->sendMessage(iter->second.commandQueue, message);
  }
}

void SubsystemBase::getMode(Mode_t* mode, Submode_t* submode) {
  *mode = this->mode;
  *submode = this->submode;
}

void SubsystemBase::setToExternalControl() { healthHelper.setHealth(EXTERNAL_CONTROL); }

void SubsystemBase::announceMode(bool recursive) {
  triggerEvent(MODE_INFO, mode, submode);
  if (recursive) {
    CommandMessage command;
    ModeMessage::setModeMessage(&command, ModeMessage::CMD_MODE_ANNOUNCE_RECURSIVELY, 0, 0);
    commandAllChildren(&command);
  }
}

void SubsystemBase::checkCommandQueue() {
  ReturnValue_t result;
  CommandMessage command;

  for (result = commandQueue->receiveMessage(&command); result == returnvalue::OK;
       result = commandQueue->receiveMessage(&command)) {
    result = healthHelper.handleHealthCommand(&command);
    if (result == returnvalue::OK) {
      continue;
    }

    result = modeHelper.handleModeCommand(&command);
    if (result == returnvalue::OK) {
      continue;
    }

    result = handleModeReply(&command);
    if (result == returnvalue::OK) {
      continue;
    }

    result = handleCommandMessage(&command);
    if (result != returnvalue::OK) {
      CommandMessage reply;
      reply.setReplyRejected(CommandMessage::UNKNOWN_COMMAND, command.getCommand());
      replyToCommand(&reply);
    }
  }
}

ReturnValue_t SubsystemBase::setHealth(HealthState health) {
  switch (health) {
    case HEALTHY:
    case EXTERNAL_CONTROL:
      healthHelper.setHealth(health);
      return returnvalue::OK;
    default:
      return INVALID_HEALTH_STATE;
  }
}

HasHealthIF::HealthState SubsystemBase::getHealth() { return healthHelper.getHealth(); }

void SubsystemBase::modeChanged() {}
