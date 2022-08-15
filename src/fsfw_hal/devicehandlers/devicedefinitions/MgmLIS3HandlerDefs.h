#ifndef MISSION_DEVICES_DEVICEDEFINITIONS_MGMLIS3HANDLERDEFS_H_
#define MISSION_DEVICES_DEVICEDEFINITIONS_MGMLIS3HANDLERDEFS_H_

#include <fsfw/datapoollocal/LocalPoolVariable.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/devicehandlers/DeviceHandlerIF.h>

#include <cstdint>

namespace MGMLIS3MDL {

enum Set { ON, OFF };
enum OpMode { LOW, MEDIUM, HIGH, ULTRA };

enum Sensitivies : uint8_t { GAUSS_4 = 4, GAUSS_8 = 8, GAUSS_12 = 12, GAUSS_16 = 16 };

/* Actually 15, we just round up a bit */
static constexpr size_t MAX_BUFFER_SIZE = 16;

/* Field data register scaling */
static constexpr uint8_t GAUSS_TO_MICROTESLA_FACTOR = 100;
static constexpr float FIELD_LSB_PER_GAUSS_4_SENS = 1.0 / 6842.0;
static constexpr float FIELD_LSB_PER_GAUSS_8_SENS = 1.0 / 3421.0;
static constexpr float FIELD_LSB_PER_GAUSS_12_SENS = 1.0 / 2281.0;
static constexpr float FIELD_LSB_PER_GAUSS_16_SENS = 1.0 / 1711.0;

static const DeviceCommandId_t READ_CONFIG_AND_DATA = 0x00;
static const DeviceCommandId_t SETUP_MGM = 0x01;
static const DeviceCommandId_t READ_TEMPERATURE = 0x02;
static const DeviceCommandId_t IDENTIFY_DEVICE = 0x03;
static const DeviceCommandId_t TEMP_SENSOR_ENABLE = 0x04;
static const DeviceCommandId_t ACCURACY_OP_MODE_SET = 0x05;

/* Number of all control registers */
static const uint8_t NR_OF_CTRL_REGISTERS = 5;
/* Number of registers in the MGM */
static const uint8_t NR_OF_REGISTERS = 19;
/* Total number of adresses for all registers */
static const uint8_t TOTAL_NR_OF_ADRESSES = 52;
static const uint8_t NR_OF_DATA_AND_CFG_REGISTERS = 14;
static const uint8_t TEMPERATURE_REPLY_LEN = 3;
static const uint8_t SETUP_REPLY_LEN = 6;

/*------------------------------------------------------------------------*/
/* Register adresses */
/*------------------------------------------------------------------------*/
/* Register adress returns identifier of device with default 0b00111101 */
static const uint8_t IDENTIFY_DEVICE_REG_ADDR = 0b00001111;
static const uint8_t DEVICE_ID = 0b00111101;  // Identifier for Device

/* Register adress to access register 1 */
static const uint8_t CTRL_REG1 = 0b00100000;
/* Register adress to access register 2 */
static const uint8_t CTRL_REG2 = 0b00100001;
/* Register adress to access register 3 */
static const uint8_t CTRL_REG3 = 0b00100010;
/* Register adress to access register 4 */
static const uint8_t CTRL_REG4 = 0b00100011;
/* Register adress to access register 5 */
static const uint8_t CTRL_REG5 = 0b00100100;

/* Register adress to access status register */
static const uint8_t STATUS_REG_IDX = 8;
static const uint8_t STATUS_REG = 0b00100111;

/* Register adress to access low byte of x-axis */
static const uint8_t X_LOWBYTE_IDX = 9;
static const uint8_t X_LOWBYTE = 0b00101000;
/* Register adress to access high byte of x-axis */
static const uint8_t X_HIGHBYTE_IDX = 10;
static const uint8_t X_HIGHBYTE = 0b00101001;
/* Register adress to access low byte of y-axis */
static const uint8_t Y_LOWBYTE_IDX = 11;
static const uint8_t Y_LOWBYTE = 0b00101010;
/* Register adress to access high byte of y-axis */
static const uint8_t Y_HIGHBYTE_IDX = 12;
static const uint8_t Y_HIGHBYTE = 0b00101011;
/* Register adress to access low byte of z-axis */
static const uint8_t Z_LOWBYTE_IDX = 13;
static const uint8_t Z_LOWBYTE = 0b00101100;
/* Register adress to access high byte of z-axis */
static const uint8_t Z_HIGHBYTE_IDX = 14;
static const uint8_t Z_HIGHBYTE = 0b00101101;

/* Register adress to access low byte of temperature sensor */
static const uint8_t TEMP_LOWBYTE = 0b00101110;
/* Register adress to access high byte of temperature sensor */
static const uint8_t TEMP_HIGHBYTE = 0b00101111;

/*------------------------------------------------------------------------*/
/* Initialize Setup Register set bits                                     */
/*------------------------------------------------------------------------*/
/* General transfer bits */
// Read=1 / Write=0 Bit
static const uint8_t RW_BIT = 7;
// Continous Read/Write Bit, increment adress
static const uint8_t MS_BIT = 6;

/* CTRL_REG1 bits */
static const uint8_t ST = 0;  // Self test enable bit, enabled = 1
// Enable rates higher than 80 Hz enabled = 1
static const uint8_t FAST_ODR = 1;
static const uint8_t DO0 = 2;      // Output data rate bit 2
static const uint8_t DO1 = 3;      // Output data rate bit 3
static const uint8_t DO2 = 4;      // Output data rate bit 4
static const uint8_t OM0 = 5;      // XY operating mode bit 5
static const uint8_t OM1 = 6;      // XY operating mode bit 6
static const uint8_t TEMP_EN = 7;  // Temperature sensor enable enabled = 1
static const uint8_t CTRL_REG1_DEFAULT =
    (1 << TEMP_EN) | (1 << OM1) | (1 << DO0) | (1 << DO1) | (1 << DO2);

/* CTRL_REG2 bits */
// reset configuration registers and user registers
static const uint8_t SOFT_RST = 2;
static const uint8_t REBOOT = 3;  // reboot memory content
static const uint8_t FSO = 5;     // full-scale selection bit 5
static const uint8_t FS1 = 6;     // full-scale selection bit 6
static const uint8_t CTRL_REG2_DEFAULT = 0;

/* CTRL_REG3 bits */
static const uint8_t MD0 = 0;  // Operating mode bit 0
static const uint8_t MD1 = 1;  // Operating mode bit 1
// SPI serial interface mode selection enabled = 3-wire-mode
static const uint8_t SIM = 2;
static const uint8_t LP = 5;  // low-power mode
static const uint8_t CTRL_REG3_DEFAULT = 0;

/* CTRL_REG4 bits */
// big/little endian data selection enabled = MSb at lower adress
static const uint8_t BLE = 1;
static const uint8_t OMZ0 = 2;  // Z operating mode bit 2
static const uint8_t OMZ1 = 3;  // Z operating mode bit 3
static const uint8_t CTRL_REG4_DEFAULT = (1 << OMZ1);

/* CTRL_REG5 bits */
static const uint8_t BDU = 6;        // Block data update
static const uint8_t FAST_READ = 7;  // Fast read enabled = 1
static const uint8_t CTRL_REG5_DEFAULT = 0;

static const uint32_t MGM_DATA_SET_ID = READ_CONFIG_AND_DATA;

enum MgmPoolIds : lp_id_t { FIELD_STRENGTHS, TEMPERATURE_CELCIUS };

class MgmPrimaryDataset : public StaticLocalDataSet<4> {
 public:
  MgmPrimaryDataset(HasLocalDataPoolIF* hkOwner) : StaticLocalDataSet(hkOwner, MGM_DATA_SET_ID) {}

  MgmPrimaryDataset(object_id_t mgmId) : StaticLocalDataSet(sid_t(mgmId, MGM_DATA_SET_ID)) {}

  /**
   * Field strenghts in uT
   */
  lp_vec_t<float, 3> fieldStrengths = lp_vec_t<float, 3>(sid.objectId, FIELD_STRENGTHS, this);
  lp_var_t<float> temperature = lp_var_t<float>(sid.objectId, TEMPERATURE_CELCIUS, this);
};

}  // namespace MGMLIS3MDL

#endif /* MISSION_DEVICES_DEVICEDEFINITIONS_MGMLIS3HANDLERDEFS_H_ */
