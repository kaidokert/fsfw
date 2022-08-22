#ifndef FRAMEWORK_CONTAINER_PLACEMENTFACTORY_H_
#define FRAMEWORK_CONTAINER_PLACEMENTFACTORY_H_

#include <utility>

#include "../storagemanager/StorageManagerIF.h"
/**
 * The Placement Factory is used to create objects at runtime in a specific pool.
 * In general, this should be avoided and it should only be used if you know what you are doing.
 * You are not allowed to use this container with a type that allocates memory internally like
 * ArrayList.
 *
 * Also, you have to check the returned pointer in generate against nullptr!
 *
 * A backend of Type StorageManagerIF must be given as a place to store the new objects.
 * Therefore ThreadSafety is only provided by your StorageManager Implementation.
 *
 * Objects must be destroyed by the user with "destroy"! Otherwise the pool will not be cleared.
 *
 * The concept is based on the placement new operator.
 *
 * @warning Do not use with any Type that allocates memory internally!
 * @ingroup container
 */
class PlacementFactory {
 public:
  PlacementFactory(StorageManagerIF* backend) : dataBackend(backend) {}

  /***
   * Generates an object of type T in the backend storage.
   *
   * @warning Do not use with any Type that allocates memory internally!
   *
   * @tparam T Type of Object
   * @param args Constructor Arguments to be passed
   * @return A pointer to the new object or a nullptr in case of failure
   */
  template <typename T, typename... Args>
  T* generate(Args&&... args) {
    store_address_t tempId;
    uint8_t* pData = nullptr;
    ReturnValue_t result = dataBackend->getFreeElement(&tempId, sizeof(T), &pData);
    if (result != returnvalue::OK) {
      return nullptr;
    }
    T* temp = new (pData) T(std::forward<Args>(args)...);
    return temp;
  }
  /***
   * Function to destroy the object allocated with generate and free space in backend.
   * This must be called by the user.
   *
   * @param thisElement Element to be destroyed
   * @return returnvalue::OK if the element was destroyed, different errors on failure
   */
  template <typename T>
  ReturnValue_t destroy(T* thisElement) {
    if (thisElement == nullptr) {
      return returnvalue::FAILED;
    }
    // Need to call destructor first, in case something was allocated by the object (shouldn't do
    // that, however).
    thisElement->~T();
    uint8_t* pointer = (uint8_t*)(thisElement);
    return dataBackend->deleteData(pointer, sizeof(T));
  }

 private:
  StorageManagerIF* dataBackend;
};

#endif /* FRAMEWORK_CONTAINER_PLACEMENTFACTORY_H_ */
