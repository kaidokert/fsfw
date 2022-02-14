#ifndef FSFW_POWER_POWERSENSOR_H_
#define FSFW_POWER_POWERSENSOR_H_

#include "fsfw/datapoollocal/StaticLocalDataSet.h"
#include "fsfw/devicehandlers/HealthDevice.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/monitoring/LimitMonitor.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/parameters/ParameterHelper.h"

class PowerController;

/**
 * @brief	Does magic.
 */
class PowerSensor : public SystemObject, public ReceivesParameterMessagesIF, public HasHealthIF {
  friend class PowerController;

 public:
  struct VariableIds {
    gp_id_t pidCurrent;
    gp_id_t pidVoltage;
    gp_id_t poolIdPower;
  };
  struct DefaultLimits {
    float currentMin;
    float currentMax;
    float voltageMin;
    float voltageMax;
  };
  struct SensorEvents {
    Event currentLow;
    Event currentHigh;
    Event voltageLow;
    Event voltageHigh;
  };
  PowerSensor(object_id_t objectId, sid_t sid, VariableIds setIds, DefaultLimits limits,
              SensorEvents events, uint16_t confirmationCount = 0);
  virtual ~PowerSensor();
  ReturnValue_t calculatePower();
  ReturnValue_t performOperation(uint8_t opCode);
  void setAllMonitorsToUnchecked();
  MessageQueueId_t getCommandQueue() const;
  ReturnValue_t initialize();
  void setDataPoolEntriesInvalid();
  float getPower();
  ReturnValue_t setHealth(HealthState health);
  HasHealthIF::HealthState getHealth();
  ReturnValue_t getParameter(uint8_t domainId, uint8_t uniqueId, ParameterWrapper *parameterWrapper,
                             const ParameterWrapper *newValues, uint16_t startAtIndex);

 private:
  MessageQueueIF *commandQueue = nullptr;
  ParameterHelper parameterHelper;
  HealthHelper healthHelper;
  // GlobDataSet set;
  StaticLocalDataSet<3> powerSensorSet;
  // Variables in
  lp_var_t<float> current;
  lp_var_t<float> voltage;
  // PIDReader<float> current;
  // PIDReader<float> voltage;
  // Variables out
  lp_var_t<float> power;
  // gp_float_t power;

  static const uint8_t MODULE_ID_CURRENT = 1;
  static const uint8_t MODULE_ID_VOLTAGE = 2;
  void checkCommandQueue();

 protected:
  LimitMonitor<float> currentLimit;
  LimitMonitor<float> voltageLimit;
};

#endif /* FSFW_POWER_POWERSENSOR_H_ */
