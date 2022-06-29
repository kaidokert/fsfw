#pragma once

#include <fsfw/serialize/SerializeIF.h>
#include <fsfw/introspection/Types.h>

#ifdef FSFW_INTROSPECTION
#include <vector>
#endif

class ParameterIF : public SerializeIF {
 public:
  virtual bool isValid() = 0;
#ifdef FSFW_INTROSPECTION

  

  virtual const char *getName() = 0;

  virtual Types::ParameterType getType() = 0;

  virtual double getFloating() = 0;
  virtual int64_t getSigned() = 0;

  virtual bool setFloating(double value) = 0;
  virtual bool setSigned(int64_t value) = 0;

  virtual double getMinFloating() = 0;
  virtual int64_t getMinSigned() = 0;

  virtual double getMaxFloating() = 0;
  virtual int64_t getMaxSigned() = 0;

  virtual std::vector<int64_t> getEnumValues() = 0;
  virtual const char *const * getEnumDescriptions() = 0;

  // ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
  //                                 Endianness streamEndianness) const override = 0;

  // size_t getSerializedSize() const override = 0;

  // ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
  //                                   Endianness streamEndianness) override = 0;

#endif
};