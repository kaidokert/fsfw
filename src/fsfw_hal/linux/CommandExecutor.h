#ifndef FSFW_SRC_FSFW_OSAL_LINUX_COMMANDEXECUTOR_H_
#define FSFW_SRC_FSFW_OSAL_LINUX_COMMANDEXECUTOR_H_

#include <poll.h>

#include <string>
#include <vector>

#include "fsfw/returnvalues/FwClassIds.h"
#include "fsfw/returnvalues/returnvalue.h"

class SimpleRingBuffer;
template <typename T>
class DynamicFIFO;

/**
 * @brief   Helper class to execute shell commands in blocking and non-blocking mode
 * @details
 * This class is able to execute processes by using the Linux popen call. It also has the
 * capability of writing the read output of a process into a provided ring buffer.
 *
 * The executor works by first loading the command which should be executed and specifying
 * whether it should be executed blocking or non-blocking. After that, execution can be started
 * with the execute command. In blocking mode, the execute command will block until the command
 * has finished
 */
class CommandExecutor {
 public:
  enum class States { IDLE, COMMAND_LOADED, PENDING };

  static constexpr uint8_t CLASS_ID = CLASS_ID::LINUX_OSAL;

  //! [EXPORT] : [COMMENT] Execution of the current command has finished
  static constexpr ReturnValue_t EXECUTION_FINISHED = returnvalue::makeCode(CLASS_ID, 0);

  //! [EXPORT] : [COMMENT] Command is pending. This will also be returned if the user tries
  //! to load another command but a command is still pending
  static constexpr ReturnValue_t COMMAND_PENDING = returnvalue::makeCode(CLASS_ID, 1);
  //! [EXPORT] : [COMMENT] Some bytes have been read from the executing process
  static constexpr ReturnValue_t BYTES_READ = returnvalue::makeCode(CLASS_ID, 2);
  //! [EXPORT] : [COMMENT] Command execution failed
  static constexpr ReturnValue_t COMMAND_ERROR = returnvalue::makeCode(CLASS_ID, 3);
  //! [EXPORT] : [COMMENT]
  static constexpr ReturnValue_t NO_COMMAND_LOADED_OR_PENDING = returnvalue::makeCode(CLASS_ID, 4);
  static constexpr ReturnValue_t PCLOSE_CALL_ERROR = returnvalue::makeCode(CLASS_ID, 6);

  /**
   * Constructor. Is initialized with maximum size of internal buffer to read data from the
   * executed process.
   * @param maxSize
   */
  explicit CommandExecutor(size_t maxSize);

  /**
   * Load a new command which should be executed
   * @param command
   * @param blocking
   * @param printOutput
   * @return
   */
  ReturnValue_t load(std::string command, bool blocking, bool printOutput = true);
  /**
   * Execute the loaded command.
   * @return
   *  - In blocking mode, it will return returnvalue::FAILED if
   *    the result of the system call was not 0. The error value can be accessed using
   *    getLastError
   *  - In non-blocking mode, this call will start
   *    the execution and then return returnvalue::OK
   */
  ReturnValue_t execute();
  /**
   * Only used in non-blocking mode. Checks the currently running command.
   * @param bytesRead Will be set to the number of bytes read, if bytes have been read
   * @return
   *  - BYTES_READ if bytes have been read from the executing process. It is recommended to call
   *    check again after this
   *  - returnvalue::OK execution is pending, but no bytes have been read from the executing process
   *  - returnvalue::FAILED if execution has failed, error value can be accessed using getLastError
   *  - EXECUTION_FINISHED if the process was executed successfully
   *  - NO_COMMAND_LOADED_OR_PENDING self-explanatory
   *  - COMMAND_ERROR internal poll error
   */
  ReturnValue_t check(bool& replyReceived);
  /**
   * Abort the current command. Should normally not be necessary, check can be used to find
   * out whether command execution was successful
   * @return returnvalue::OK
   */
  ReturnValue_t close();

  States getCurrentState() const;
  int getLastError() const;
  void printLastError(std::string funcName) const;

  /**
   * Assign a ring buffer and a FIFO which will be filled by the executor with the output
   * read from the started process
   * @param ringBuffer
   * @param sizesFifo
   */
  void setRingBuffer(SimpleRingBuffer* ringBuffer, DynamicFIFO<uint16_t>* sizesFifo);

  /**
   * Reset the executor. This calls close internally and then reset the state machine so new
   * commands can be loaded and executed
   */
  void reset();

 private:
  std::string currentCmd;
  bool blocking = true;
  FILE* currentCmdFile = nullptr;
  int currentFd = 0;
  bool printOutput = true;
  std::vector<char> readVec;
  struct pollfd waiter {};
  SimpleRingBuffer* ringBuffer = nullptr;
  DynamicFIFO<uint16_t>* sizesFifo = nullptr;

  States state = States::IDLE;
  int lastError = 0;

  ReturnValue_t executeBlocking();
};

#endif /* FSFW_SRC_FSFW_OSAL_LINUX_COMMANDEXECUTOR_H_ */
