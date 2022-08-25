#pragma once

#include <cstddef>
#include <cstdint>

class EnumIF {
 public:
  virtual ~EnumIF() {}
  virtual int64_t getValue() const = 0;
  virtual bool isValid() const = 0;
  virtual size_t getSize() const = 0;
  virtual size_t getIndex(int64_t value) const = 0;
  virtual const int64_t *getElements() const = 0;
  virtual const char *const *getDescriptions() const = 0;
  virtual const char *getDescription() const = 0;
};