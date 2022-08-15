#include "fsfw/memory/MemoryHelper.h"

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/memory/MemoryMessage.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serialize/EndianConverter.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

MemoryHelper::MemoryHelper(HasMemoryIF* workOnThis, MessageQueueIF* useThisQueue)
    : workOnThis(workOnThis),
      queueToUse(useThisQueue),
      ipcAddress(),
      lastCommand(CommandMessage::CMD_NONE),
      busy(false) {}

ReturnValue_t MemoryHelper::handleMemoryCommand(CommandMessage* message) {
  lastSender = message->getSender();
  lastCommand = message->getCommand();
  if (busy) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "MemHelper: Busy!" << std::endl;
#endif
  }
  switch (lastCommand) {
    case MemoryMessage::CMD_MEMORY_DUMP:
      handleMemoryCheckOrDump(message);
      return returnvalue::OK;
    case MemoryMessage::CMD_MEMORY_LOAD:
      handleMemoryLoad(message);
      return returnvalue::OK;
    case MemoryMessage::CMD_MEMORY_CHECK:
      handleMemoryCheckOrDump(message);
      return returnvalue::OK;
    default:
      lastCommand = CommandMessage::CMD_NONE;
      return UNKNOWN_CMD;
  }
}

void MemoryHelper::completeLoad(ReturnValue_t errorCode, const uint8_t* dataToCopy,
                                const size_t size, uint8_t* copyHere) {
  busy = false;
  switch (errorCode) {
    case HasMemoryIF::DO_IT_MYSELF:
      busy = true;
      return;
    case HasMemoryIF::POINTS_TO_MEMORY:
      memcpy(copyHere, dataToCopy, size);
      break;
    case HasMemoryIF::POINTS_TO_VARIABLE:
      EndianConverter::convertBigEndian(copyHere, dataToCopy, size);
      break;
    case HasMemoryIF::ACTIVITY_COMPLETED:
    case returnvalue::OK:
      break;
    default:
      ipcStore->deleteData(ipcAddress);
      CommandMessage reply;
      MemoryMessage::setMemoryReplyFailed(&reply, errorCode, MemoryMessage::CMD_MEMORY_LOAD);
      queueToUse->sendMessage(lastSender, &reply);
      return;
  }
  // Only reached on success
  CommandMessage reply(CommandMessage::REPLY_COMMAND_OK, 0, 0);
  queueToUse->sendMessage(lastSender, &reply);
  ipcStore->deleteData(ipcAddress);
}

void MemoryHelper::completeDump(ReturnValue_t errorCode, const uint8_t* dataToCopy,
                                const size_t size) {
  busy = false;
  CommandMessage reply;
  MemoryMessage::setMemoryReplyFailed(&reply, errorCode, lastCommand);
  switch (errorCode) {
    case HasMemoryIF::DO_IT_MYSELF:
      busy = true;
      return;
    case returnvalue::OK:
    case HasMemoryIF::POINTS_TO_MEMORY:
    case HasMemoryIF::POINTS_TO_VARIABLE:
      //"data" must be valid pointer!
      if (errorCode == HasMemoryIF::POINTS_TO_VARIABLE) {
        EndianConverter::convertBigEndian(reservedSpaceInIPC, dataToCopy, size);
      } else {
        memcpy(reservedSpaceInIPC, dataToCopy, size);
      }
      /* NO BREAK falls through*/
    case HasMemoryIF::ACTIVITY_COMPLETED:
      switch (lastCommand) {
        case MemoryMessage::CMD_MEMORY_DUMP: {
          MemoryMessage::setMemoryDumpReply(&reply, ipcAddress);
          break;
        }
        case MemoryMessage::CMD_MEMORY_CHECK: {
          uint16_t crc = CRC::crc16ccitt(reservedSpaceInIPC, size);
          // Delete data immediately, was temporary.
          ipcStore->deleteData(ipcAddress);
          MemoryMessage::setMemoryCheckReply(&reply, crc);
          break;
        }
        default:
          // This should never happen!
          // Is it ok to send message? Otherwise: return;
          ipcStore->deleteData(ipcAddress);
          reply.setParameter(STATE_MISMATCH);
          break;
      }
      break;
    case HasMemoryIF::DUMP_NOT_SUPPORTED:
      if (lastCommand == MemoryMessage::CMD_MEMORY_CHECK) {
        MemoryMessage::setMemoryCheckReply(&reply, 0);
        MemoryMessage::setCrcReturnValue(&reply, HasMemoryIF::DUMP_NOT_SUPPORTED);
      }
      ipcStore->deleteData(ipcAddress);
      break;
    default:
      // Reply is already set to REJECTED.
      ipcStore->deleteData(ipcAddress);
      break;
  }
  if (queueToUse->sendMessage(lastSender, &reply) != returnvalue::OK) {
    reply.clear();
  }
}

void MemoryHelper::swapMatrixCopy(uint8_t* out, const uint8_t* in, size_t totalSize,
                                  uint8_t datatypeSize) {
  if (totalSize % datatypeSize != 0) {
    return;
  }

  while (totalSize > 0) {
    EndianConverter::convertBigEndian(out, in, datatypeSize);
    out += datatypeSize;
    in += datatypeSize;
    totalSize -= datatypeSize;
  }
}

MemoryHelper::~MemoryHelper() {
  // Nothing to destroy
}

void MemoryHelper::handleMemoryLoad(CommandMessage* message) {
  uint32_t address = MemoryMessage::getAddress(message);
  ipcAddress = MemoryMessage::getStoreID(message);
  const uint8_t* p_data = NULL;
  uint8_t* dataPointer = NULL;
  size_t size = 0;
  ReturnValue_t returnCode = ipcStore->getData(ipcAddress, &p_data, &size);
  if (returnCode == returnvalue::OK) {
    returnCode = workOnThis->handleMemoryLoad(address, p_data, size, &dataPointer);
    completeLoad(returnCode, p_data, size, dataPointer);
  } else {
    // At least inform sender.
    CommandMessage reply;
    MemoryMessage::setMemoryReplyFailed(&reply, returnCode, MemoryMessage::CMD_MEMORY_LOAD);
    queueToUse->sendMessage(lastSender, &reply);
  }
}

void MemoryHelper::handleMemoryCheckOrDump(CommandMessage* message) {
  uint32_t address = MemoryMessage::getAddress(message);
  uint32_t size = MemoryMessage::getLength(message);
  uint8_t* dataPointer = NULL;
  ReturnValue_t returnCode = ipcStore->getFreeElement(&ipcAddress, size, &reservedSpaceInIPC);
  if (returnCode == returnvalue::OK) {
    returnCode = workOnThis->handleMemoryDump(address, size, &dataPointer, reservedSpaceInIPC);
    completeDump(returnCode, dataPointer, size);
  } else {
    CommandMessage reply;
    MemoryMessage::setMemoryReplyFailed(&reply, returnCode, lastCommand);
    queueToUse->sendMessage(lastSender, &reply);
  }
}

ReturnValue_t MemoryHelper::initialize(MessageQueueIF* queueToUse_) {
  if (queueToUse_ == nullptr) {
    return returnvalue::FAILED;
  }
  this->queueToUse = queueToUse_;
  return initialize();
}

ReturnValue_t MemoryHelper::initialize() {
  ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
  if (ipcStore != nullptr) {
    return returnvalue::OK;
  } else {
    return returnvalue::FAILED;
  }
}
