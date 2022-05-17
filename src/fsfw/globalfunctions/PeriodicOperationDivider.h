#ifndef FSFW_GLOBALFUNCTIONS_PERIODICOPERATIONDIVIDER_H_
#define FSFW_GLOBALFUNCTIONS_PERIODICOPERATIONDIVIDER_H_

#include <cstdint>

/**
 * @brief	Lightweight helper class to facilitate periodic operation with
 * 			decreased frequencies.
 * @details
 * This class is useful to perform operations which have to be performed
 * with a reduced frequency, like debugging printouts in high periodic tasks
 * or low priority operations.
 */
class PeriodicOperationDivider {
 public:
  /**
   * Initialize with the desired divider and specify whether the internal
   * counter will be reset automatically.
   * @param divider Value of 0 or 1 will cause #check and #checkAndIncrement to always return
   * true
   * @param resetAutomatically
   */
  PeriodicOperationDivider(uint32_t divider, bool resetAutomatically = true);

  /**
   * Check whether operation is necessary. If an operation is necessary and the class has been
   * configured to be reset automatically, the counter will be reset to 1 automatically
   *
   * @return
   * -@c true if the counter is larger or equal to the divider
   * -@c false otherwise
   */
  bool checkAndIncrement();

  /**
   * Checks whether an operation is necessary. This function will not increment the counter.
   * @return
   * -@c true if the counter is larger or equal to the divider
   * -@c false otherwise
   */
  bool check();

  /**
   * Can be used to reset the counter to 1 manually
   */
  void resetCounter();
  uint32_t getCounter() const;

  /**
   * Can be used to set a new divider value.
   * @param newDivider
   */
  void setDivider(uint32_t newDivider);
  uint32_t getDivider() const;

 private:
  bool resetAutomatically = true;
  uint32_t counter = 1;
  uint32_t divider = 0;
};

#endif /* FSFW_GLOBALFUNCTIONS_PERIODICOPERATIONDIVIDER_H_ */
