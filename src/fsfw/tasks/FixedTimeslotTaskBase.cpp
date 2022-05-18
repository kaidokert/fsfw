#include "FixedTimeslotTaskBase.h"

#include "fsfw/objectmanager/ObjectManager.h"

FixedTimeslotTaskBase::FixedTimeslotTaskBase(TaskPeriod period_,
                                             TaskDeadlineMissedFunction dlmFunc_)
    : pollingSeqTable(getPeriodMs()), period(period_), dlmFunc(dlmFunc_) {}
uint32_t FixedTimeslotTaskBase::getPeriodMs() const { return static_cast<uint32_t>(period * 1000); }

bool FixedTimeslotTaskBase::isEmpty() const { return pollingSeqTable.isEmpty(); }

ReturnValue_t FixedTimeslotTaskBase::checkSequence() { return pollingSeqTable.checkSequence(); }

ReturnValue_t FixedTimeslotTaskBase::addSlot(object_id_t componentId, uint32_t slotTimeMs,
                                             int8_t executionStep) {
  auto* executableObject = ObjectManager::instance()->get<ExecutableObjectIF>(componentId);
  if (executableObject != nullptr) {
    pollingSeqTable.addSlot(componentId, slotTimeMs, executionStep, executableObject, this);
    return HasReturnvaluesIF::RETURN_OK;
  }

#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::error << "Component 0x" << std::hex << std::setw(8) << std::setfill('0') << componentId
             << std::setfill(' ') << " not found, not adding it to PST" << std::dec << std::endl;
#else
  sif::printError("Component 0x%08x not found, not adding it to PST\n");
#endif
  return HasReturnvaluesIF::RETURN_FAILED;
}
