#include "Action.h"

#include <fsfw/serialize/SerializeAdapter.h>

#undef Action

#ifdef FSFW_INTROSPECTION

Action::Action() {}

void Action::setEnum(EnumIF *theEnum) {
  id = theEnum->getValue();
  name = theEnum->getDescription();
}

const char *Action::getName() { return name; }
#else
Action::Action(ActionId_t id) : id(id) {}
#endif
ActionId_t Action::getId() { return id; }

void Action::registerParameter(ParameterIF *parameter) { parameterList.push_back(parameter); }

std::vector<ParameterIF *> const *Action::getParameters() const { return &parameterList; }

size_t Action::getSerializedSize() const {
  size_t size = SerializeAdapter::getSerializedSize(&id);
  for (auto parameter : *getParameters()) {
    size += parameter->getSerializedSize();
  }
  return size;
}

ReturnValue_t Action::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                Endianness streamEndianness) const {
  ReturnValue_t result = SerializeAdapter::serialize(&id, buffer, size, maxSize, streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  for (auto parameter : *getParameters()) {
    result = parameter->serialize(buffer, size, maxSize, streamEndianness);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      return result;
    }
  }
  return result;
}

ReturnValue_t Action::deSerialize(const uint8_t **buffer, size_t *size,
                                  Endianness streamEndianness) {
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;/* TODO not needed as must have been read before to find this action = SerializeAdapter::deSerialize(&id, buffer, size, streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }*/
  for (auto parameter : *getParameters()) {
    result = parameter->deSerialize(buffer, size, streamEndianness);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      return result;
    }
  }
  return result;
}