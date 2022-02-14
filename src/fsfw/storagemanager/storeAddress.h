#ifndef FSFW_STORAGEMANAGER_STOREADDRESS_H_
#define FSFW_STORAGEMANAGER_STOREADDRESS_H_

#include <cstdint>

namespace storeId {
static constexpr uint32_t INVALID_STORE_ADDRESS = 0xffffffff;
}

/**
 * This union defines the type that identifies where a data packet is
 * stored in the store. It comprises of a raw part to read it as raw value and
 * a structured part to use it in pool-like stores.
 */
union store_address_t {
  /**
   * Default Constructor, initializing to INVALID_ADDRESS
   */
  store_address_t() : raw(storeId::INVALID_STORE_ADDRESS) {}
  /**
   * Constructor to create an address object using the raw address
   *
   * @param rawAddress
   */
  store_address_t(uint32_t rawAddress) : raw(rawAddress) {}

  /**
   * Constructor to create an address object using pool
   * and packet indices
   *
   * @param poolIndex
   * @param packetIndex
   */
  store_address_t(uint16_t poolIndex, uint16_t packetIndex)
      : poolIndex(poolIndex), packetIndex(packetIndex) {}
  /**
   * A structure with two elements to access the store address pool-like.
   */
  struct {
    /**
     * The index in which pool the packet lies.
     */
    uint16_t poolIndex;
    /**
     * The position in the chosen pool.
     */
    uint16_t packetIndex;
  };
  /**
   * Alternative access to the raw value.
   */
  uint32_t raw;

  bool operator==(const store_address_t& other) const { return raw == other.raw; }
};

#endif /* FSFW_STORAGEMANAGER_STOREADDRESS_H_ */
