#include "FixedTimeslotTaskBase.h"

#include "fsfw/objectmanager/ObjectManager.h"

FixedTimeslotTaskBase::FixedTimeslotTaskBase(TaskPeriod period_,
                                             TaskDeadlineMissedFunction dlmFunc_)
    : period(period_), pollingSeqTable(getPeriodMs()), dlmFunc(dlmFunc_) {}
uint32_t FixedTimeslotTaskBase::getPeriodMs() const { return static_cast<uint32_t>(period * 1000); }

bool FixedTimeslotTaskBase::isEmpty() const { return pollingSeqTable.isEmpty(); }

ReturnValue_t FixedTimeslotTaskBase::checkSequence() { return pollingSeqTable.checkSequence(); }

ReturnValue_t FixedTimeslotTaskBase::addSlot(object_id_t execId, ExecutableObjectIF* execObj,
                                             uint32_t slotTimeMs, int8_t executionStep) {
  if (execObj == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Component 0x" << std::hex << std::setw(8) << std::setfill('0') << execObj
               << std::setfill(' ') << " not found, not adding it to PST" << std::dec << std::endl;
#else
    sif::printError("Component 0x%08x not found, not adding it to PST\n");
#endif
    return returnvalue::FAILED;
  }
  pollingSeqTable.addSlot(execId, slotTimeMs, executionStep, execObj, this);
  return returnvalue::OK;
}
