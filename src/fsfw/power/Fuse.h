#ifndef FSFW_POWER_FUSE_H_
#define FSFW_POWER_FUSE_H_

#include <list>

#include "../datapoollocal/StaticLocalDataSet.h"
#include "../devicehandlers/HealthDevice.h"
#include "../monitoring/AbsLimitMonitor.h"
#include "../parameters/ParameterHelper.h"
#include "../returnvalues/returnvalue.h"
#include "PowerComponentIF.h"
#include "PowerSwitchIF.h"
namespace Factory {
void setStaticFrameworkObjectIds();
}

class Fuse : public SystemObject,
             public HasHealthIF,
             public ReceivesParameterMessagesIF,
             public SerializeIF {
  friend void(Factory::setStaticFrameworkObjectIds)();

 private:
  static constexpr float RESIDUAL_POWER =
      0.005 * 28.5;  //!< This is the upper limit of residual power lost by fuses and switches.
                     //!< Worst case is Fuse and one of two switches on. See PCDU ICD 1.9 p29 bottom
 public:
  struct VariableIds {
    gp_id_t pidVoltage;
    gp_id_t pidCurrent;
    gp_id_t pidState;
    gp_id_t poolIdPower;
  };

  static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::PCDU_1;
  //! PSS detected that current on a fuse is totally out of bounds.
  static const Event FUSE_CURRENT_HIGH = MAKE_EVENT(1, severity::LOW);
  //! PSS detected a fuse that went off.
  static const Event FUSE_WENT_OFF = MAKE_EVENT(2, severity::LOW);
  //! PSS detected a fuse that violates its limits.
  static const Event POWER_ABOVE_HIGH_LIMIT = MAKE_EVENT(4, severity::LOW);
  //! PSS detected a fuse that violates its limits.
  static const Event POWER_BELOW_LOW_LIMIT = MAKE_EVENT(5, severity::LOW);

  typedef std::list<PowerComponentIF *> DeviceList;
  Fuse(object_id_t fuseObjectId, uint8_t fuseId, sid_t variableSet, VariableIds ids,
       float maxCurrent, uint16_t confirmationCount = 2);
  virtual ~Fuse();
  void addDevice(PowerComponentIF *set);
  float getPower();

  bool isPowerValid();

  ReturnValue_t check();
  uint8_t getFuseId() const;
  ReturnValue_t initialize() override;
  DeviceList devices;
  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          SerializeIF::Endianness streamEndianness) const override;
  size_t getSerializedSize() const override;
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            SerializeIF::Endianness streamEndianness) override;
  void setAllMonitorsToUnchecked();
  ReturnValue_t performOperation(uint8_t opCode);
  MessageQueueId_t getCommandQueue() const;
  void setDataPoolEntriesInvalid();
  ReturnValue_t setHealth(HealthState health);
  HasHealthIF::HealthState getHealth();

  ReturnValue_t getParameter(uint8_t domainId, uint8_t uniqueId, ParameterWrapper *parameterWrapper,
                             const ParameterWrapper *newValues, uint16_t startAtIndex);

 private:
  uint8_t oldFuseState;
  uint8_t fuseId;
  PowerSwitchIF *powerIF = nullptr;  // could be static in our case.
  AbsLimitMonitor<float> currentLimit;
  class PowerMonitor : public MonitorReporter<float> {
   public:
    template <typename... Args>
    PowerMonitor(Args... args) : MonitorReporter<float>(std::forward<Args>(args)...) {}
    ReturnValue_t checkPower(float sample, float lowerLimit, float upperLimit);
    void sendTransitionEvent(float currentValue, ReturnValue_t state) {}
  };
  PowerMonitor powerMonitor;
  StaticLocalDataSet<3> set;

  lp_var_t<float> voltage;
  lp_var_t<float> current;
  lp_var_t<uint8_t> state;

  lp_var_t<float> power;
  MessageQueueIF *commandQueue = nullptr;
  ParameterHelper parameterHelper;
  HealthHelper healthHelper;
  static object_id_t powerSwitchId;
  void calculatePowerLimits(float *low, float *high);
  void calculateFusePower();
  void checkFuseState();
  void reportEvents(Event event);
  void checkCommandQueue();

  bool areSwitchesOfComponentOn(DeviceList::iterator iter);
};

#endif /* FSFW_POWER_FUSE_H_ */
