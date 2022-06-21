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
    // TODO: Hex formatting
    FSFW_LOGE("Component {} not found, not adding it to PST\n");
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  pollingSeqTable.addSlot(execId, slotTimeMs, executionStep, execObj, this);
  return HasReturnvaluesIF::RETURN_OK;
}
