#ifndef FSFW_TASKS_FIXEDSLOTSEQUENCE_H_
#define FSFW_TASKS_FIXEDSLOTSEQUENCE_H_

#include <set>

#include "FixedSequenceSlot.h"
#include "fsfw/objectmanager/SystemObject.h"

/**
 * @brief	This class is the representation of a
 * 			Polling Sequence Table in software.
 * @details
 * The FixedSlotSequence object maintains the dynamic execution of
 * objects with stricter timing requirements for the FixedTimeslotTask.
 *
 * The main idea is to create a list of executable objects (for example
 * device handlers), to announce all handlers to the polling sequence and to
 * maintain a list of polling slot objects.
 * This slot list represents the Polling Sequence Table in software.
 *
 * Each polling slot contains information to indicate when and
 * which executable object shall be executed within a given polling period.
 * When adding a slot, a pointer to the executing task, a pointer to the
 * executable object and a step number can be passed. The step number will be
 * passed to the periodic handler.
 * The sequence is executed by iterating through the slot sequence and
 * executing the executable object in the correct timeslot.
 */
class FixedSlotSequence {
 public:
  using SlotList = std::multiset<FixedSequenceSlot>;
  using SlotListIter = std::multiset<FixedSequenceSlot>::iterator;
  using CustomCheckFunc = ReturnValue_t (*)(const SlotList&, void* args);
  /**
   * @brief	The constructor of the FixedSlotSequence object.
   * @param	setLength	The period length, expressed in ms.
   */
  explicit FixedSlotSequence(uint32_t setLengthMs);

  /**
   * @brief	The destructor of the FixedSlotSequence object.
   * @details
   * The destructor frees all allocated memory by iterating through the
   * slotList and deleting all allocated resources.
   */
  virtual ~FixedSlotSequence();

  /**
   * @brief	This is a method to add an PollingSlot object to slotList.
   *
   * @details
   * Here, a polling slot object is added to the slot list. It is appended
   * to the end of the list. The list is currently NOT reordered.
   * Afterwards, the iterator current is set to the beginning of the list.
   * 	@param handlerId ID of the object to add
   * 	@param setTime
   * 	Value between (0 to 1) * slotLengthMs, when a FixedTimeslotTask
   * 	will be called inside the slot period.
   * 	@param setSequenceId
   * 	ID which can be used to distinguish different task operations. This
   * 	value will be passed to the executable function.
   * 	@param
   * 	@param
   */
  void addSlot(object_id_t handlerId, uint32_t setTime, int8_t setSequenceId,
               ExecutableObjectIF* executableObject, PeriodicTaskIF* executingTask);

  /**
   * @brief	Checks if the current slot shall be executed immediately
   * 			after the one before.
   * @details
   * This allows to distinguish between grouped and separated handlers.
   * @return 	- @c true if the slot has the same polling time as the previous
   * 			- @c false else
   */
  bool slotFollowsImmediately();

  /**
   * @brief	This method returns the time until the next software
   * 			component is invoked.
   *
   * @details
   * This method is vitally important for the operation of the PST.
   * By fetching the polling time of the current slot and that of the
   * next one (or the first one, if the list end is reached)
   * it calculates and returns the interval in milliseconds within
   * which the handler execution shall take place.
   * If the next slot has the same time as the current one, it is ignored
   * until a slot with different time or the end of the PST is found.
   */
  uint32_t getIntervalToNextSlotMs();

  /**
   * @brief	This method returns the time difference between the current
   * 			slot and the previous slot
   *
   * @details
   * This method is vitally important for the operation of the PST.
   * By fetching the polling time of the current slot and that of the previous
   * one (or the last one, if the slot is the first one) it calculates and
   * returns the interval in milliseconds that the handler execution shall
   * be delayed.
   */
  uint32_t getIntervalToPreviousSlotMs();

  /**
   * @brief	This method returns the length of this FixedSlotSequence instance.
   */
  [[nodiscard]] uint32_t getLengthMs() const;

  /**
   * @brief	The method to execute the device handler entered in the current
   * 			PollingSlot object.
   *
   * @details
   * Within this method the device handler object to be executed is chosen by
   * looking up the handler address of the current slot in the handlerMap.
   * Either the device handler's talkToInterface or its listenToInterface
   * method is invoked, depending on the isTalking flag of the polling slot.
   * After execution the iterator current is increased or, by reaching the
   * end of slotList, reset to the beginning.
   */
  void executeAndAdvance();

  /**
   * @brief	An iterator that indicates the current polling slot to execute.
   *
   * @details	This is an iterator for slotList and always points to the
   * 			polling slot which is executed next.
   */
  SlotListIter current;

  /**
   * @brief   Check and initialize slot list.
   * @details
   * Checks if timing is ok (must be ascending) and if all handlers were found.
   * @return
   *  - SLOT_LIST_EMPTY if the slot list is empty
   */
  [[nodiscard]] ReturnValue_t checkSequence() const;

  /**
   * @brief   A custom check can be injected for the respective slot list.
   * @details
   * This can be used by the developer to check the validity of a certain
   * sequence. The function will be run in the #checkSequence function.
   * The general check will be continued for now if the custom check function
   * fails but a diagnostic debug output will be given.
   * @param customCheckFunction
   *
   */
  void addCustomCheck(CustomCheckFunc func, void* userArgs);

  /**
   * @brief 	Perform any initialization steps required after the executing
   * 			task has been created. This function should be called from the
   * 			executing task!
   * @return
   */
  [[nodiscard]] ReturnValue_t intializeSequenceAfterTaskCreation() const;

  [[nodiscard]] bool isEmpty() const;

 protected:
  /**
   * @brief	This list contains all PollingSlot objects, defining order and
   * 			execution time of the device handler objects.
   *
   * @details
   * The slot list is a std:list object that contains all created
   * PollingSlot instances. They are NOT ordered automatically, so by
   * adding entries, the correct order needs to be ensured. By iterating
   * through this list the polling sequence is executed. Two entries with
   * identical polling times are executed immediately one after another.
   */
  SlotList slotList;

  CustomCheckFunc customChecker = nullptr;
  void* customCheckArgs = nullptr;

  uint32_t lengthMs;
};

#endif /* FSFW_TASKS_FIXEDSLOTSEQUENCE_H_ */
