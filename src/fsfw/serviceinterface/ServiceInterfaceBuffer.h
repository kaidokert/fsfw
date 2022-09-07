#ifndef FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACEBUFFER_H_
#define FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACEBUFFER_H_

#include "fsfw/FSFW.h"
#include "fsfw/returnvalues/returnvalue.h"

#if FSFW_CPP_OSTREAM_ENABLED == 1

#include <iomanip>
#include <iostream>
#include <sstream>

/**
 * @brief 	This is the underlying stream buffer which implements the
 * 			streambuf class and overloads the overflow() and sync() methods
 * @details
 * This class is used to modify the output of the stream, for example by adding.
 * It also calls the char printing function which is implemented in the
 * board supply package (BSP).
 */
class ServiceInterfaceBuffer : public std::streambuf {
  friend class ServiceInterfaceStream;

 public:
  static constexpr uint8_t MAX_PREAMBLE_SIZE = 40;

  ServiceInterfaceBuffer(std::string setMessage, bool addCrToPreamble, bool buffered,
                         bool errStream, uint16_t port);

 protected:
  bool isActive;
  //! This is called when buffer becomes full. If
  //! buffer is not used, then this is called every
  //! time when characters are put to stream.
  int overflow(int c = Traits::eof()) override;

  //! This function is called when stream is flushed,
  //! for example when std::endl is put to stream.
  int sync(void) override;

  bool isBuffered() const;

 private:
  //! For additional message information
  std::string logMessage;
  std::string preamble;

#if FSFW_COLORED_OUTPUT == 1
  std::string colorPrefix;
  void setAsciiColorPrefix(std::string colorPrefix);
#endif

  // For EOF detection
  typedef std::char_traits<char> Traits;

  //! This is useful for some terminal programs which do not have
  //! implicit carriage return with newline characters.
  bool addCrToPreamble;

  //! Specifies whether the stream operates in buffered or unbuffered mode.
  bool buffered;
  //! This specifies to print to stderr and work in unbuffered mode.
  bool errStream;

  //! Needed for buffered mode.
  static size_t const BUF_SIZE = fsfwconfig::FSFW_PRINT_BUFFER_SIZE;
  char buf[BUF_SIZE];

  //! In this function, the characters are parsed.
  void putChars(char const* begin, char const* end);

  std::string* getPreamble(size_t* preambleSize = nullptr);

  bool crAdditionEnabled() const;
};

#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */

#endif /* FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACEBUFFER_H_ */
