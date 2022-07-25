#ifndef FSFW_OBJECTMANAGER_OBJECTMANAGER_H_
#define FSFW_OBJECTMANAGER_OBJECTMANAGER_H_

#include <map>

#include "ObjectManagerIF.h"
#include "SystemObjectIF.h"

/**
 * @brief	This class implements a global object manager.
 * @details	This manager handles a list of available objects with system-wide
 * 			relevance, such as device handlers, and TM/TC services. Objects can
 * 			be inserted, removed and retrieved from the list. In addition, the
 * 			class holds a so-called factory, that creates and inserts new
 * 			objects if they are not already in the list. This feature automates
 * 			most of the system initialization.
 * 			As the system is static after initialization, no new objects are
 * 			created or inserted into the list after startup.
 * @ingroup system_objects
 * @author	Bastian Baetz
 */
class ObjectManager : public ObjectManagerIF {
 public:
  using produce_function_t = void (*)(void* args);

  /**
   * Returns the single instance of TaskFactory.
   * The implementation of #instance is found in its subclasses.
   * Thus, we choose link-time variability of the  instance.
   */
  static ObjectManager* instance();

  void setObjectFactoryFunction(produce_function_t prodFunc, void* args);

  template <typename T>
  T* get(object_id_t id);

  /**
   *  @brief  In the class's destructor, all objects in the list are deleted.
   */
  ~ObjectManager() override;
  ReturnValue_t insert(object_id_t id, SystemObjectIF* object) override;
  ReturnValue_t remove(object_id_t id) override;
  void initialize() override;
  void printList() override;

 protected:
  SystemObjectIF* getSystemObject(object_id_t id) override;
  /**
   * @brief   This attribute is initialized with the factory function
   *          that creates new objects.
   * @details The function is called if an object was requested with
   *          getSystemObject, but not found in objectList.
   * @param   The id of the object to be created.
   * @return  Returns a pointer to the newly created object or NULL.
   */
  produce_function_t objectFactoryFunction = nullptr;
  void* factoryArgs = nullptr;

 private:
  ObjectManager();

  /**
   * @brief	This is the map of all initialized objects in the manager.
   * @details	Objects in the List must inherit the SystemObjectIF.
   */
  std::map<object_id_t, SystemObjectIF*> objectList;
  static ObjectManager* objManagerInstance;
};

// Documentation can be found in the class method declaration above
template <typename T>
T* ObjectManager::get(object_id_t id) {
  SystemObjectIF* temp = this->getSystemObject(id);
  return dynamic_cast<T*>(temp);
}

#endif /* FSFW_OBJECTMANAGER_OBJECTMANAGER_H_ */
