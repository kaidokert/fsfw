#ifndef MISSION_DEVICES_DEVICEDEFINITIONS_MGMHANDLERRM3100DEFINITIONS_H_
#define MISSION_DEVICES_DEVICEDEFINITIONS_MGMHANDLERRM3100DEFINITIONS_H_

#include <fsfw/datapoollocal/LocalPoolVariable.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/devicehandlers/DeviceHandlerIF.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>

#include <cstdint>

namespace RM3100 {

/* Actually 10, we round up a little bit */
static constexpr size_t MAX_BUFFER_SIZE = 12;

static constexpr uint8_t READ_MASK = 0x80;

/*----------------------------------------------------------------------------*/
/* CMM Register */
/*----------------------------------------------------------------------------*/
static constexpr uint8_t SET_CMM_CMZ = 1 << 6;
static constexpr uint8_t SET_CMM_CMY = 1 << 5;
static constexpr uint8_t SET_CMM_CMX = 1 << 4;
static constexpr uint8_t SET_CMM_DRDM = 1 << 2;
static constexpr uint8_t SET_CMM_START = 1;
static constexpr uint8_t CMM_REGISTER = 0x01;

static constexpr uint8_t CMM_VALUE =
    SET_CMM_CMZ | SET_CMM_CMY | SET_CMM_CMX | SET_CMM_DRDM | SET_CMM_START;

/*----------------------------------------------------------------------------*/
/* Cycle count register */
/*----------------------------------------------------------------------------*/
// Default value (200)
static constexpr uint8_t CYCLE_COUNT_VALUE = 0xC8;

static constexpr float DEFAULT_GAIN = static_cast<float>(CYCLE_COUNT_VALUE) / 100 * 38;
static constexpr uint8_t CYCLE_COUNT_START_REGISTER = 0x04;

/*----------------------------------------------------------------------------*/
/* TMRC register */
/*----------------------------------------------------------------------------*/
static constexpr uint8_t TMRC_150HZ_VALUE = 0x94;
static constexpr uint8_t TMRC_75HZ_VALUE = 0x95;
static constexpr uint8_t TMRC_DEFAULT_37HZ_VALUE = 0x96;

static constexpr uint8_t TMRC_REGISTER = 0x0B;
static constexpr uint8_t TMRC_DEFAULT_VALUE = TMRC_DEFAULT_37HZ_VALUE;

static constexpr uint8_t MEASUREMENT_REG_START = 0x24;
static constexpr uint8_t BIST_REGISTER = 0x33;
static constexpr uint8_t DATA_READY_VAL = 0b10000000;
static constexpr uint8_t STATUS_REGISTER = 0x34;
static constexpr uint8_t REVID_REGISTER = 0x36;

// Range in Microtesla. 1 T equals 10000 Gauss (for comparison with LIS3 MGM)
static constexpr uint16_t RANGE = 800;

static constexpr DeviceCommandId_t READ_DATA = 0;

static constexpr DeviceCommandId_t CONFIGURE_CMM = 1;
static constexpr DeviceCommandId_t READ_CMM = 2;

static constexpr DeviceCommandId_t CONFIGURE_TMRC = 3;
static constexpr DeviceCommandId_t READ_TMRC = 4;

static constexpr DeviceCommandId_t CONFIGURE_CYCLE_COUNT = 5;
static constexpr DeviceCommandId_t READ_CYCLE_COUNT = 6;

class CycleCountCommand : public SerialLinkedListAdapter<SerializeIF> {
 public:
  CycleCountCommand(bool oneCycleCount = true) : oneCycleCount(oneCycleCount) {
    setLinks(oneCycleCount);
  }

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override {
    ReturnValue_t result = SerialLinkedListAdapter::deSerialize(buffer, size, streamEndianness);
    if (oneCycleCount) {
      cycleCountY = cycleCountX;
      cycleCountZ = cycleCountX;
    }
    return result;
  }

  SerializeElement<uint16_t> cycleCountX;
  SerializeElement<uint16_t> cycleCountY;
  SerializeElement<uint16_t> cycleCountZ;

 private:
  void setLinks(bool oneCycleCount) {
    setStart(&cycleCountX);
    if (not oneCycleCount) {
      cycleCountX.setNext(&cycleCountY);
      cycleCountY.setNext(&cycleCountZ);
    }
  }

  bool oneCycleCount;
};

static constexpr uint32_t MGM_DATASET_ID = READ_DATA;

enum MgmPoolIds : lp_id_t { FIELD_STRENGTHS };

class Rm3100PrimaryDataset : public StaticLocalDataSet<3> {
 public:
  Rm3100PrimaryDataset(HasLocalDataPoolIF* hkOwner) : StaticLocalDataSet(hkOwner, MGM_DATASET_ID) {}

  Rm3100PrimaryDataset(object_id_t mgmId) : StaticLocalDataSet(sid_t(mgmId, MGM_DATASET_ID)) {}

  /**
   * Field strenghts in uT
   */
  lp_vec_t<float, 3> fieldStrengths = lp_vec_t<float, 3>(sid.objectId, FIELD_STRENGTHS, this);
};

}  // namespace RM3100

#endif /* MISSION_DEVICES_DEVICEDEFINITIONS_MGMHANDLERRM3100DEFINITIONS_H_ */
