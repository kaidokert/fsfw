#include "ObjectManager.h"

#include "fsfw/serviceinterface.h"

#if FSFW_CPP_OSTREAM_ENABLED == 1
#include <iomanip>
#endif
#include <cstdlib>

ObjectManager* ObjectManager::objManagerInstance = nullptr;

ObjectManager* ObjectManager::instance() {
  if (objManagerInstance == nullptr) {
    objManagerInstance = new ObjectManager();
  }
  return objManagerInstance;
}

void ObjectManager::setObjectFactoryFunction(produce_function_t objFactoryFunc, void* factoryArgs) {
  this->objectFactoryFunction = objFactoryFunc;
  this->factoryArgs = factoryArgs;
}

ObjectManager::ObjectManager() {}

ObjectManager::~ObjectManager() {
  for (auto const& iter : objectList) {
    delete iter.second;
  }
}

ReturnValue_t ObjectManager::insert(object_id_t id, SystemObjectIF* object) {
  auto returnPair = objectList.emplace(id, object);
  if (returnPair.second) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    // sif::debug << "ObjectManager::insert: Object " << std::hex
    //            << (int)id << std::dec << " inserted." << std::endl;
#endif
    return this->RETURN_OK;
  } else {
    FSFW_FLOGET("ObjectManager::insert: Object ID {:#08x} is already in use\nTerminating program\n",
                static_cast<uint32_t>(id));
    // This is very severe and difficult to handle in other places.
    std::exit(INSERTION_FAILED);
  }
}

ReturnValue_t ObjectManager::remove(object_id_t id) {
  if (this->getSystemObject(id) != NULL) {
    this->objectList.erase(id);
#if FSFW_CPP_OSTREAM_ENABLED == 1
    // sif::debug << "ObjectManager::removeObject: Object " << std::hex
    //            << (int)id << std::dec << " removed." << std::endl;
#endif
    return RETURN_OK;
  } else {
    FSFW_FLOGW("removeObject: Requested object {:#08x} not found\n", id);
    return NOT_FOUND;
  }
}

SystemObjectIF* ObjectManager::getSystemObject(object_id_t id) {
  auto listIter = this->objectList.find(id);
  if (listIter == this->objectList.end()) {
    return nullptr;
  } else {
    return listIter->second;
  }
}

void ObjectManager::initialize() {
  if (objectFactoryFunction != nullptr) {
    objectFactoryFunction(factoryArgs);
  }
  ReturnValue_t result = RETURN_FAILED;
  uint32_t errorCount = 0;
  for (auto const& it : objectList) {
    result = it.second->initialize();
    if (result != RETURN_OK) {
      object_id_t var = it.first;
      FSFW_FLOGWT("initialize: Object {:#08x} failed to initialize with code {:#04x}\n", var,
                  result);
      errorCount++;
    }
  }
  if (errorCount > 0) {
    FSFW_FLOGWT("{}", "initialize: Counted failed initializations\n");
  }
  // Init was successful. Now check successful interconnections.
  errorCount = 0;
  for (auto const& it : objectList) {
    result = it.second->checkObjectConnections();
    if (result != RETURN_OK) {
      FSFW_FLOGE("initialize: Object {:#08x} connection check failed with code {:#04x}\n", it.first,
                 result);
      errorCount++;
    }
  }
  if (errorCount > 0) {
    FSFW_FLOGE("{}", "ObjectManager::ObjectManager: Counted {} failed connection checks\n",
               errorCount);
  }
}

void ObjectManager::printList() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::info("ObjectManager: Object List contains:\n");
  for (auto const& it : objectList) {
    sif::info("{:#08x} | {:#08x}\n", it.first, fmt::ptr(it.second));
  }
#endif
}
