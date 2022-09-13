#ifndef MISSION_DEVICES_MGMLIS3MDLHANDLER_H_
#define MISSION_DEVICES_MGMLIS3MDLHANDLER_H_

#include "devicedefinitions/MgmLIS3HandlerDefs.h"
#include "fsfw/devicehandlers/DeviceHandlerBase.h"
#include "fsfw/globalfunctions/PeriodicOperationDivider.h"

class PeriodicOperationDivider;

/**
 * @brief   Device handler object for the LIS3MDL 3-axis magnetometer
 *          by STMicroeletronics
 * @details
 * Datasheet can be found online by googling LIS3MDL.
 * Flight manual:
 * https://egit.irs.uni-stuttgart.de/redmine/projects/eive-flight-manual/wiki/LIS3MDL_MGM
 * @author  L. Loidold, R. Mueller
 */
class MgmLIS3MDLHandler : public DeviceHandlerBase {
 public:
  enum class CommunicationStep { DATA, TEMPERATURE };

  static const uint8_t INTERFACE_ID = CLASS_ID::MGM_LIS3MDL;
  static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::MGM_LIS3MDL;
  // Notifies a command to change the setup parameters
  static const Event CHANGE_OF_SETUP_PARAMETER = MAKE_EVENT(0, severity::LOW);

  MgmLIS3MDLHandler(uint32_t objectId, object_id_t deviceCommunication, CookieIF *comCookie,
                    uint32_t transitionDelay);
  virtual ~MgmLIS3MDLHandler();

  void enablePeriodicPrintouts(bool enable, uint8_t divider);
  /**
   * Set the absolute limit for the values on the axis in microtesla. The dataset values will
   * be marked as invalid if that limit is exceeded
   * @param xLimit
   * @param yLimit
   * @param zLimit
   */
  void setAbsoluteLimits(float xLimit, float yLimit, float zLimit);
  void setToGoToNormalMode(bool enable);

 protected:
  /** DeviceHandlerBase overrides */
  void doShutDown() override;
  void doStartUp() override;
  void doTransition(Mode_t modeFrom, Submode_t subModeFrom) override;
  virtual uint32_t getTransitionDelayMs(Mode_t from, Mode_t to) override;
  ReturnValue_t buildCommandFromCommand(DeviceCommandId_t deviceCommand, const uint8_t *commandData,
                                        size_t commandDataLen) override;
  ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t *id) override;
  ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t *id) override;
  ReturnValue_t scanForReply(const uint8_t *start, size_t len, DeviceCommandId_t *foundId,
                             size_t *foundLen) override;
  /**
   * This implementation is tailored towards space applications and will flag values larger
   * than 100 microtesla on X,Y and 150 microtesla on Z as invalid
   * @param id
   * @param packet
   * @return
   */
  virtual ReturnValue_t interpretDeviceReply(DeviceCommandId_t id, const uint8_t *packet) override;
  void fillCommandAndReplyMap() override;
  void modeChanged(void) override;
  ReturnValue_t initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
                                        LocalDataPoolManager &poolManager) override;

 private:
  MGMLIS3MDL::MgmPrimaryDataset dataset;
  // Length a single command SPI answer
  static const uint8_t SINGLE_COMMAND_ANSWER_LEN = 2;

  uint32_t transitionDelay;
  // Single SPI command has 2 bytes, first for adress, second for content
  size_t singleComandSize = 2;
  // Has the size for all adresses of the lis3mdl + the continous write bit
  uint8_t commandBuffer[MGMLIS3MDL::NR_OF_DATA_AND_CFG_REGISTERS + 1];

  float absLimitX = 100;
  float absLimitY = 100;
  float absLimitZ = 150;

  /**
   * We want to save the registers we set, so we dont have to read the
   * registers when we want to change something.
   * --> everytime we change set a register we have to save it
   */
  uint8_t registers[MGMLIS3MDL::NR_OF_CTRL_REGISTERS];

  uint8_t statusRegister = 0;
  bool goToNormalMode = false;

  enum class InternalState {
    STATE_NONE,
    STATE_FIRST_CONTACT,
    STATE_SETUP,
    STATE_CHECK_REGISTERS,
    STATE_NORMAL
  };

  InternalState internalState = InternalState::STATE_NONE;
  CommunicationStep communicationStep = CommunicationStep::DATA;
  bool commandExecuted = false;

  PoolEntry<float> mgmXYZ = PoolEntry<float>(3);
  PoolEntry<float> temperature = PoolEntry<float>();
  /*------------------------------------------------------------------------*/
  /* Device specific commands and variables */
  /*------------------------------------------------------------------------*/
  /**
   * Sets the read bit for the command
   * @param single command to set the read-bit at
   * @param boolean to select a continuous read bit, default = false
   */
  uint8_t readCommand(uint8_t command, bool continuousCom = false);

  /**
   * Sets the write bit for the command
   * @param single command to set the write-bit at
   * @param boolean to select a continuous write bit, default = false
   */
  uint8_t writeCommand(uint8_t command, bool continuousCom = false);

  /**
   * This Method gets the full scale for the measurement range
   * e.g.: +- 4 gauss. See p.25 datasheet.
   *  @return The ReturnValue does not contain the sign of the value
   */
  MGMLIS3MDL::Sensitivies getSensitivity(uint8_t ctrlReg2);

  /**
   * The 16 bit value needs to be multiplied with a sensitivity factor
   * which depends on the sensitivity configuration
   *
   * @param sens Configured sensitivity of the LIS3 device
   * @return Multiplication factor to get the sensor value from raw data.
   */
  float getSensitivityFactor(MGMLIS3MDL::Sensitivies sens);

  /**
   * This Command detects the device ID
   */
  ReturnValue_t identifyDevice();

  virtual void setupMgm();

  /*------------------------------------------------------------------------*/
  /* Non normal commands */
  /*------------------------------------------------------------------------*/
  /**
   * Enables/Disables the integrated Temperaturesensor
   * @param commandData On or Off
   * @param length of the commandData: has to be 1
   */
  virtual ReturnValue_t enableTemperatureSensor(const uint8_t *commandData, size_t commandDataLen);

  /**
   * Sets the accuracy of the measurement of the axis. The noise is changing.
   * @param commandData LOW, MEDIUM, HIGH, ULTRA
   * @param length of the command, has to be 1
   */
  virtual ReturnValue_t setOperatingMode(const uint8_t *commandData, size_t commandDataLen);

  /**
   * We always update all registers together, so this method updates
   * the rawpacket and rawpacketLen, so we just manipulate the local
   * saved register
   *
   */
  ReturnValue_t prepareCtrlRegisterWrite();

  bool periodicPrintout = false;
  PeriodicOperationDivider debugDivider = PeriodicOperationDivider(3);
};

#endif /* MISSION_DEVICES_MGMLIS3MDLHANDLER_H_ */
