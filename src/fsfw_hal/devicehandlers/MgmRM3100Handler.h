#ifndef MISSION_DEVICES_MGMRM3100HANDLER_H_
#define MISSION_DEVICES_MGMRM3100HANDLER_H_

#include "devicedefinitions/MgmRM3100HandlerDefs.h"
#include "fsfw/devicehandlers/DeviceHandlerBase.h"
#include "fsfw/globalfunctions/PeriodicOperationDivider.h"

/**
 * @brief 	Device Handler for the RM3100 geomagnetic magnetometer sensor
 *          (https://www.pnicorp.com/rm3100/)
 * @details
 * Flight manual:
 * https://egit.irs.uni-stuttgart.de/redmine/projects/eive-flight-manual/wiki/RM3100_MGM
 */
class MgmRM3100Handler : public DeviceHandlerBase {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::MGM_RM3100;

  //! [EXPORT] : [COMMENT] P1: TMRC value which was set, P2: 0
  static constexpr Event tmrcSet = event::makeEvent(SUBSYSTEM_ID::MGM_RM3100, 0x00, severity::INFO);

  //! [EXPORT] : [COMMENT] Cycle counter set. P1: First two bytes new Cycle Count X
  //! P1: Second two bytes new Cycle Count Y
  //! P2: New cycle count Z
  static constexpr Event cycleCountersSet =
      event::makeEvent(SUBSYSTEM_ID::MGM_RM3100, 0x01, severity::INFO);

  MgmRM3100Handler(object_id_t objectId, object_id_t deviceCommunication, CookieIF *comCookie,
                   uint32_t transitionDelay);
  virtual ~MgmRM3100Handler();

  void enablePeriodicPrintouts(bool enable, uint8_t divider);
  /**
   * Configure device handler to go to normal mode after startup immediately
   * @param enable
   */
  void setToGoToNormalMode(bool enable);

 protected:
  /* DeviceHandlerBase overrides */
  ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t *id) override;
  void doStartUp() override;
  void doShutDown() override;
  ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t *id) override;
  ReturnValue_t buildCommandFromCommand(DeviceCommandId_t deviceCommand, const uint8_t *commandData,
                                        size_t commandDataLen) override;
  ReturnValue_t scanForReply(const uint8_t *start, size_t len, DeviceCommandId_t *foundId,
                             size_t *foundLen) override;
  ReturnValue_t interpretDeviceReply(DeviceCommandId_t id, const uint8_t *packet) override;

  void fillCommandAndReplyMap() override;
  void modeChanged(void) override;
  virtual uint32_t getTransitionDelayMs(Mode_t from, Mode_t to) override;
  ReturnValue_t initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
                                        LocalDataPoolManager &poolManager) override;

 private:
  enum class InternalState {
    NONE,
    CONFIGURE_CMM,
    READ_CMM,
    // The cycle count states are propably not going to be used because
    // the default cycle count will be used.
    STATE_CONFIGURE_CYCLE_COUNT,
    STATE_READ_CYCLE_COUNT,
    STATE_CONFIGURE_TMRC,
    STATE_READ_TMRC,
    NORMAL
  };
  InternalState internalState = InternalState::NONE;
  bool commandExecuted = false;
  RM3100::Rm3100PrimaryDataset primaryDataset;

  uint8_t commandBuffer[10];
  uint8_t commandBufferLen = 0;

  uint8_t cmmRegValue = RM3100::CMM_VALUE;
  uint8_t tmrcRegValue = RM3100::TMRC_DEFAULT_VALUE;
  uint16_t cycleCountRegValueX = RM3100::CYCLE_COUNT_VALUE;
  uint16_t cycleCountRegValueY = RM3100::CYCLE_COUNT_VALUE;
  uint16_t cycleCountRegValueZ = RM3100::CYCLE_COUNT_VALUE;
  float scaleFactorX = 1.0 / RM3100::DEFAULT_GAIN;
  float scaleFactorY = 1.0 / RM3100::DEFAULT_GAIN;
  float scaleFactorZ = 1.0 / RM3100::DEFAULT_GAIN;

  bool goToNormalModeAtStartup = false;
  uint32_t transitionDelay;
  PoolEntry<float> mgmXYZ = PoolEntry<float>(3);

  ReturnValue_t handleCycleCountConfigCommand(DeviceCommandId_t deviceCommand,
                                              const uint8_t *commandData, size_t commandDataLen);
  ReturnValue_t handleCycleCommand(bool oneCycleValue, const uint8_t *commandData,
                                   size_t commandDataLen);

  ReturnValue_t handleTmrcConfigCommand(DeviceCommandId_t deviceCommand, const uint8_t *commandData,
                                        size_t commandDataLen);

  ReturnValue_t handleDataReadout(const uint8_t *packet);

  bool periodicPrintout = false;
  PeriodicOperationDivider debugDivider = PeriodicOperationDivider(3);
};

#endif /* MISSION_DEVICEHANDLING_MGMRM3100HANDLER_H_ */
