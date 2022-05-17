#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLDEFINITIONS_H_
#define FSFW_DATAPOOLLOCAL_LOCALPOOLDEFINITIONS_H_

#include <cstdint>
#include <map>

#include "../datapool/PoolEntryIF.h"
#include "../objectmanager/SystemObjectIF.h"
#include "../objectmanager/frameworkObjects.h"

/**
 * @brief   Type definition for local pool entries.
 */
using lp_id_t = uint32_t;

namespace localpool {
static constexpr uint32_t INVALID_LPID = -1;

static constexpr uint8_t INTERFACE_ID = CLASS_ID::LOCAL_POOL_OWNER_IF;

static constexpr ReturnValue_t POOL_ENTRY_NOT_FOUND = MAKE_RETURN_CODE(0x00);
static constexpr ReturnValue_t POOL_ENTRY_TYPE_CONFLICT = MAKE_RETURN_CODE(0x01);

/** This is the core data structure of the local data pools. Users should insert all desired
pool variables, using the std::map interface. */
using DataPool = std::map<lp_id_t, PoolEntryIF*>;
using DataPoolMapIter = DataPool::iterator;

}  // namespace localpool

/**
 * Used as a unique identifier for data sets. Consists of 4 byte object ID and 4 byte set ID.
 */
union sid_t {
  static constexpr uint64_t INVALID_SID = -1;
  static constexpr uint32_t INVALID_OBJECT_ID = objects::NO_OBJECT;
  static constexpr uint32_t INVALID_SET_ID = -1;

  sid_t() : raw(INVALID_SID) {}

  sid_t(object_id_t objectId, uint32_t setId) : objectId(objectId), ownerSetId(setId) {}

  struct {
    object_id_t objectId;
    /**
     * A generic 32 bit ID to identify unique HK packets for a single
     * object. For example, the DeviceCommandId_t is used for
     * DeviceHandlers
     */
    uint32_t ownerSetId;
  };
  /**
   * Alternative access to the raw value. This is also the size of the type.
   */
  uint64_t raw;

  bool notSet() const { return raw == INVALID_SID; }

  bool operator==(const sid_t& other) const { return raw == other.raw; }

  bool operator!=(const sid_t& other) const { return not(raw == other.raw); }
};

/**
 * Used as a global unique identifier for local pool variables. Consists of 4 byte object ID
 * and 4 byte local pool ID.
 */
union gp_id_t {
  static constexpr uint64_t INVALID_GPID = -1;
  static constexpr uint32_t INVALID_OBJECT_ID = objects::NO_OBJECT;
  static constexpr uint32_t INVALID_LPID = localpool::INVALID_LPID;

  gp_id_t() : raw(INVALID_GPID) {}

  gp_id_t(object_id_t objectId, lp_id_t localPoolId)
      : objectId(objectId), localPoolId(localPoolId) {}

  struct {
    object_id_t objectId;
    lp_id_t localPoolId;
  };

  uint64_t raw;

  bool notSet() const { return raw == INVALID_GPID; }

  bool operator==(const gp_id_t& other) const { return raw == other.raw; }

  bool operator!=(const gp_id_t& other) const { return not(raw == other.raw); }
};

#endif /* FSFW_DATAPOOLLOCAL_LOCALPOOLDEFINITIONS_H_ */
