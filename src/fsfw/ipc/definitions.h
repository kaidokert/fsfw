#ifndef FSFW_SRC_FSFW_IPC_DEFINITIONS_H_
#define FSFW_SRC_FSFW_IPC_DEFINITIONS_H_

#include <fsfw/objectmanager/SystemObjectIF.h>
#include <fsfw/objectmanager/frameworkObjects.h>

struct MqArgs {
  MqArgs(){};
  MqArgs(object_id_t objectId, void* args = nullptr) : objectId(objectId), args(args) {}
  object_id_t objectId = objects::NO_OBJECT;
  void* args = nullptr;
};

#endif /* FSFW_SRC_FSFW_IPC_DEFINITIONS_H_ */
