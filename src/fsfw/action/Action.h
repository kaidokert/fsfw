#pragma once

#include <stdint.h>

#include <vector>

#include <fsfw/serialize/SerializeIF.h>
#include "ActionMessage.h"
#include "ParameterIF.h"


#ifdef FSFW_INTROSPECTION
#include "../introspection/Enum.h"
#endif

class Action: public SerializeIF {
 public:
#ifdef FSFW_INTROSPECTION
  Action();
  void setEnum(EnumIF* id);
  const char *getName();
#else
  Action(ActionId_t id);
#endif
  ActionId_t getId();

  MessageQueueId_t commandedBy;

  virtual ReturnValue_t handle() = 0;

  void registerParameter(ParameterIF *parameter);

  std::vector<ParameterIF *> const *getParameters() const;

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                  Endianness streamEndianness) const override;

  size_t getSerializedSize() const override;

  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                                    Endianness streamEndianness) override;

 private:
  ActionId_t id;
  
#ifdef FSFW_INTROSPECTION
  const char *name;
#endif
  std::vector<ParameterIF *> parameterList;
};