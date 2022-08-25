#pragma once

#include <fsfw/introspection/EnumIF.h>

class ModeDefinitionHelper {
 public:
#ifdef FSFW_INTROSPECTION
  ModeDefinitionHelper(EnumIF *mode, EnumIF *submode) : mode(mode), submode(submode) {}
#else
  ModeDefinitionHelper(void *mode, void *submode) {};
#endif
  template <typename Mode, typename Submode>
  static ModeDefinitionHelper create() {
#ifdef FSFW_INTROSPECTION
    EnumIF *mode = new Mode();
    EnumIF *submode = new Submode();
    return ModeDefinitionHelper(mode, submode);
#else
    return ModeDefinitionHelper(nullptr, nullptr);
#endif
  }

  void free() {
#ifdef FSFW_INTROSPECTION
    delete mode;
    delete submode;
#endif
  }
#ifdef FSFW_INTROSPECTION
  EnumIF *mode;
  EnumIF *submode;
#endif
};