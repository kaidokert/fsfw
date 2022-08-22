#include "TestAssembly.h"

#include <fsfw/objectmanager/ObjectManager.h>

TestAssembly::TestAssembly(object_id_t objectId, object_id_t parentId, object_id_t testDevice0,
                           object_id_t testDevice1)
    : AssemblyBase(objectId, parentId),
      deviceHandler0Id(testDevice0),
      deviceHandler1Id(testDevice1) {
  ModeListEntry newModeListEntry;
  newModeListEntry.setObject(testDevice0);
  newModeListEntry.setMode(MODE_OFF);
  newModeListEntry.setSubmode(SUBMODE_NONE);

  commandTable.insert(newModeListEntry);

  newModeListEntry.setObject(testDevice1);
  newModeListEntry.setMode(MODE_OFF);
  newModeListEntry.setSubmode(SUBMODE_NONE);

  commandTable.insert(newModeListEntry);
}

TestAssembly::~TestAssembly() {}

ReturnValue_t TestAssembly::commandChildren(Mode_t mode, Submode_t submode) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::info << "TestAssembly: Received command to go to mode " << mode << " submode "
            << (int)submode << std::endl;
#else
  sif::printInfo("TestAssembly: Received command to go to mode %d submode %d\n", mode, submode);
#endif
  ReturnValue_t result = returnvalue::OK;
  if (mode == MODE_OFF) {
    commandTable[0].setMode(MODE_OFF);
    commandTable[0].setSubmode(SUBMODE_NONE);
    commandTable[1].setMode(MODE_OFF);
    commandTable[1].setSubmode(SUBMODE_NONE);
  } else if (mode == DeviceHandlerIF::MODE_NORMAL) {
    if (submode == submodes::SINGLE) {
      commandTable[0].setMode(MODE_OFF);
      commandTable[0].setSubmode(SUBMODE_NONE);
      commandTable[1].setMode(MODE_OFF);
      commandTable[1].setSubmode(SUBMODE_NONE);
      // We try to prefer 0 here but we try to switch to 1 even if it might fail
      if (isDeviceAvailable(deviceHandler0Id)) {
        if (childrenMap[deviceHandler0Id].mode == MODE_ON) {
          commandTable[0].setMode(mode);
          commandTable[0].setSubmode(SUBMODE_NONE);
        } else {
          commandTable[0].setMode(MODE_ON);
          commandTable[0].setSubmode(SUBMODE_NONE);
          result = NEED_SECOND_STEP;
        }
      } else {
        if (childrenMap[deviceHandler1Id].mode == MODE_ON) {
          commandTable[1].setMode(mode);
          commandTable[1].setSubmode(SUBMODE_NONE);
        } else {
          commandTable[1].setMode(MODE_ON);
          commandTable[1].setSubmode(SUBMODE_NONE);
          result = NEED_SECOND_STEP;
        }
      }
    } else {
      // Dual Mode Normal
      if (childrenMap[deviceHandler0Id].mode == MODE_ON) {
        commandTable[0].setMode(mode);
        commandTable[0].setSubmode(SUBMODE_NONE);
      } else {
        commandTable[0].setMode(MODE_ON);
        commandTable[0].setSubmode(SUBMODE_NONE);
        result = NEED_SECOND_STEP;
      }
      if (childrenMap[deviceHandler1Id].mode == MODE_ON) {
        commandTable[1].setMode(mode);
        commandTable[1].setSubmode(SUBMODE_NONE);
      } else {
        commandTable[1].setMode(MODE_ON);
        commandTable[1].setSubmode(SUBMODE_NONE);
        result = NEED_SECOND_STEP;
      }
    }
  } else {
    // Mode ON
    if (submode == submodes::SINGLE) {
      commandTable[0].setMode(MODE_OFF);
      commandTable[0].setSubmode(SUBMODE_NONE);
      commandTable[1].setMode(MODE_OFF);
      commandTable[1].setSubmode(SUBMODE_NONE);
      // We try to prefer 0 here but we try to switch to 1 even if it might fail
      if (isDeviceAvailable(deviceHandler0Id)) {
        commandTable[0].setMode(MODE_ON);
        commandTable[0].setSubmode(SUBMODE_NONE);
      } else {
        commandTable[1].setMode(MODE_ON);
        commandTable[1].setSubmode(SUBMODE_NONE);
      }
    } else {
      commandTable[0].setMode(MODE_ON);
      commandTable[0].setSubmode(SUBMODE_NONE);
      commandTable[1].setMode(MODE_ON);
      commandTable[1].setSubmode(SUBMODE_NONE);
    }
  }

  HybridIterator<ModeListEntry> iter(commandTable.begin(), commandTable.end());
  executeTable(iter);
  return result;
}

ReturnValue_t TestAssembly::isModeCombinationValid(Mode_t mode, Submode_t submode) {
  switch (mode) {
    case MODE_OFF:
      if (submode == SUBMODE_NONE) {
        return returnvalue::OK;
      } else {
        return INVALID_SUBMODE;
      }
    case DeviceHandlerIF::MODE_NORMAL:
    case MODE_ON:
      if (submode < 3) {
        return returnvalue::OK;
      } else {
        return INVALID_SUBMODE;
      }
  }
  return INVALID_MODE;
}

ReturnValue_t TestAssembly::initialize() {
  ReturnValue_t result = AssemblyBase::initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  handler0 = ObjectManager::instance()->get<TestDevice>(deviceHandler0Id);
  handler1 = ObjectManager::instance()->get<TestDevice>(deviceHandler1Id);
  if ((handler0 == nullptr) or (handler1 == nullptr)) {
    return returnvalue::FAILED;
  }

  handler0->setParentQueue(this->getCommandQueue());
  handler1->setParentQueue(this->getCommandQueue());

  result = registerChild(deviceHandler0Id);
  if (result != returnvalue::OK) {
    return result;
  }
  result = registerChild(deviceHandler1Id);
  if (result != returnvalue::OK) {
    return result;
  }
  return result;
}

ReturnValue_t TestAssembly::checkChildrenStateOn(Mode_t wantedMode, Submode_t wantedSubmode) {
  if (submode == submodes::DUAL) {
    for (const auto& info : childrenMap) {
      if (info.second.mode != wantedMode or info.second.mode != wantedSubmode) {
        return NOT_ENOUGH_CHILDREN_IN_CORRECT_STATE;
      }
    }
    return returnvalue::OK;
  } else if (submode == submodes::SINGLE) {
    for (const auto& info : childrenMap) {
      if (info.second.mode == wantedMode and info.second.mode != wantedSubmode) {
        return returnvalue::OK;
      }
    }
  }
  return INVALID_SUBMODE;
}

bool TestAssembly::isDeviceAvailable(object_id_t object) {
  if (healthHelper.healthTable->getHealth(object) == HasHealthIF::HEALTHY) {
    return true;
  } else {
    return false;
  }
}
