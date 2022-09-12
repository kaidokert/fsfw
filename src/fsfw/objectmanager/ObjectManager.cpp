#include "fsfw/objectmanager/ObjectManager.h"

#include "fsfw/serviceinterface/ServiceInterface.h"

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

ObjectManager::ObjectManager() = default;

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
    return returnvalue::OK;
  } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "ObjectManager::insert: Object ID " << std::hex << static_cast<uint32_t>(id)
               << std::dec << " is already in use!" << std::endl;
    sif::error << "Terminating program" << std::endl;
#else
    sif::printError("ObjectManager::insert: Object ID 0x%08x is already in use!\n",
                    static_cast<unsigned int>(id));
    sif::printError("Terminating program");
#endif
    // This is very severe and difficult to handle in other places.
    std::exit(INSERTION_FAILED);
  }
}

ReturnValue_t ObjectManager::remove(object_id_t id) {
  if (this->getSystemObject(id) != nullptr) {
    this->objectList.erase(id);
#if FSFW_CPP_OSTREAM_ENABLED == 1
    // sif::debug << "ObjectManager::removeObject: Object " << std::hex
    //            << (int)id << std::dec << " removed." << std::endl;
#endif
    return returnvalue::OK;
  } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "ObjectManager::removeObject: Requested object " << std::hex << (int)id
               << std::dec << " not found." << std::endl;
#endif
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
  if (objectFactoryFunction == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "ObjectManager::initialize: Passed produceObjects "
                  "functions is nullptr!"
               << std::endl;
#else
    sif::printError("ObjectManager::initialize: Passed produceObjects functions is nullptr!\n");
#endif
    return;
  }
  objectFactoryFunction(factoryArgs);
  ReturnValue_t result = returnvalue::FAILED;
  uint32_t errorCount = 0;
  for (auto const& it : objectList) {
    result = it.second->initialize();
    if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      object_id_t var = it.first;
      sif::error << "ObjectManager::initialize: Object 0x" << std::hex << std::setw(8)
                 << std::setfill('0') << var
                 << " failed to "
                    "initialize with code 0x"
                 << result << std::dec << std::setfill(' ') << std::endl;
#endif
      errorCount++;
    }
  }
  if (errorCount > 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "ObjectManager::ObjectManager: Counted " << errorCount
               << " failed initializations." << std::endl;
#endif
  }
  // Init was successful. Now check successful interconnections.
  errorCount = 0;
  for (auto const& it : objectList) {
    result = it.second->checkObjectConnections();
    if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "ObjectManager::ObjectManager: Object 0x" << std::hex << (int)it.first
                 << " connection check failed with code 0x" << result << std::dec << std::endl;
#endif
      errorCount++;
    }
  }
  if (errorCount > 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "ObjectManager::ObjectManager: Counted " << errorCount
               << " failed connection checks." << std::endl;
#endif
  }
}

void ObjectManager::printList() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "ObjectManager: Object List contains:" << std::endl;
  for (auto const& it : objectList) {
    sif::debug << std::hex << it.first << " | " << it.second << std::endl;
  }
#endif
}
