#ifndef FSFW_SERVICEINTERFACE_SERVICEINTERFACEDEFINTIONS_H_
#define FSFW_SERVICEINTERFACE_SERVICEINTERFACEDEFINTIONS_H_

namespace sif {

enum class OutputTypes { OUT_INFO, OUT_DEBUG, OUT_WARNING, OUT_ERROR };

static const char* const ANSI_COLOR_RED = "\x1b[31m";
static const char* const ANSI_COLOR_GREEN = "\x1b[32m";
static const char* const ANSI_COLOR_YELLOW = "\x1b[33m";
static const char* const ANSI_COLOR_BLUE = "\x1b[34m";
static const char* const ANSI_COLOR_MAGENTA = "\x1b[35m";
static const char* const ANSI_COLOR_CYAN = "\x1b[36m";
static const char* const ANSI_COLOR_RESET = "\x1b[0m";

}  // namespace sif

#endif /* FSFW_SERVICEINTERFACE_SERVICEINTERFACEDEFINTIONS_H_ */
