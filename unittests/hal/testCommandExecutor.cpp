#include <unistd.h>

#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <iostream>

#include "fsfw/container/DynamicFIFO.h"
#include "fsfw/container/SimpleRingBuffer.h"
#include "fsfw/platform.h"
#include "fsfw/serviceinterface.h"
#include "fsfw_hal/linux/CommandExecutor.h"
#include "tests/TestsConfig.h"

#ifdef PLATFORM_UNIX

static const char TEST_FILE_NAME[] = "/tmp/fsfw-unittest-test.txt";

TEST_CASE("Command Executor", "[hal][linux]") {
  // Check blocking mode first
  CommandExecutor cmdExecutor(1024);
  std::string cmd = "echo \"test\" >> " + std::string(TEST_FILE_NAME);
  REQUIRE(cmdExecutor.getCurrentState() == CommandExecutor::States::IDLE);
  ReturnValue_t result = cmdExecutor.load(cmd, true, true);
  REQUIRE(cmdExecutor.getCurrentState() == CommandExecutor::States::COMMAND_LOADED);
  REQUIRE(result == returnvalue::OK);
  REQUIRE(cmdExecutor.execute() == returnvalue::OK);
  // Check that file exists with contents
  std::ifstream file(TEST_FILE_NAME);
  std::string line;
  std::getline(file, line);
  CHECK(line == "test");
  std::remove(TEST_FILE_NAME);
  REQUIRE(cmdExecutor.getCurrentState() == CommandExecutor::States::IDLE);

  // Now check non-blocking mode
  SimpleRingBuffer outputBuffer(524, true);
  DynamicFIFO<uint16_t> sizesFifo(12);
  cmdExecutor.setRingBuffer(&outputBuffer, &sizesFifo);

  result = cmdExecutor.load("echo \"Hello World\"", false, false);
  REQUIRE(result == returnvalue::OK);
  cmdExecutor.execute();
  bool bytesHaveBeenRead = false;
  size_t limitIdx = 0;
  while (result != CommandExecutor::EXECUTION_FINISHED) {
    limitIdx++;
    result = cmdExecutor.check(bytesHaveBeenRead);
    REQUIRE(result != CommandExecutor::COMMAND_ERROR);
    usleep(500);
    REQUIRE(limitIdx < 500);
  }

  limitIdx = 0;
  CHECK(bytesHaveBeenRead == true);
  CHECK(result == CommandExecutor::EXECUTION_FINISHED);
  uint16_t readBytes = 0;
  sizesFifo.retrieve(&readBytes);
  REQUIRE(readBytes == 12);
  REQUIRE(outputBuffer.getAvailableReadData() == 12);
  uint8_t readBuffer[32] = {};
  REQUIRE(outputBuffer.readData(readBuffer, 12) == returnvalue::OK);
  std::string readString(reinterpret_cast<char*>(readBuffer));
  std::string cmpString = "Hello World\n";
  CHECK(readString == cmpString);
  outputBuffer.deleteData(12, true);

  // Issues with CI/CD
#if FSFW_CICD_BUILD == 0
  // Test more complex command
  result = cmdExecutor.load("ping -c 1 localhost", false, false);
  REQUIRE(cmdExecutor.getCurrentState() == CommandExecutor::States::COMMAND_LOADED);
  REQUIRE(cmdExecutor.execute() == returnvalue::OK);
  REQUIRE(cmdExecutor.getCurrentState() == CommandExecutor::States::PENDING);
  limitIdx = 0;
  while (result != CommandExecutor::EXECUTION_FINISHED) {
    limitIdx++;
    result = cmdExecutor.check(bytesHaveBeenRead);
    REQUIRE(result != CommandExecutor::COMMAND_ERROR);
    // This ensures that the tests do not block indefinitely
    usleep(500);
    REQUIRE(limitIdx < 500);
  }
  limitIdx = 0;
  CHECK(bytesHaveBeenRead == true);
  CHECK(result == CommandExecutor::EXECUTION_FINISHED);
  REQUIRE(cmdExecutor.getCurrentState() == CommandExecutor::States::IDLE);
  readBytes = 0;
  sizesFifo.retrieve(&readBytes);
  uint8_t largerReadBuffer[1024] = {};
  // That's about the size of the reply
  bool beTrue = (readBytes > 100) and (readBytes < 400);
  if (not beTrue) {
    size_t readLen = outputBuffer.getAvailableReadData();
    if (readLen > sizeof(largerReadBuffer) - 1) {
      readLen = sizeof(largerReadBuffer) - 1;
    }
    outputBuffer.readData(largerReadBuffer, readLen);
    std::string readString(reinterpret_cast<char*>(largerReadBuffer));
    std::cerr << "Catch2 tag cmd-exec: Read " << readBytes << ": " << std::endl;
    std::cerr << readString << std::endl;
  }
  REQUIRE(beTrue);
  outputBuffer.readData(largerReadBuffer, readBytes);
  // You can also check this output in the debugger
  std::string allTheReply(reinterpret_cast<char*>(largerReadBuffer));
  // I am just going to assume that this string is the same across ping implementations
  // of different Linux systems
  REQUIRE(allTheReply.find("PING localhost") != std::string::npos);
#endif

  // Now check failing command
  result = cmdExecutor.load("false", false, false);
  REQUIRE(result == returnvalue::OK);
  result = cmdExecutor.execute();
  REQUIRE(result == returnvalue::OK);
  while (result != CommandExecutor::EXECUTION_FINISHED and result != returnvalue::FAILED) {
    limitIdx++;
    result = cmdExecutor.check(bytesHaveBeenRead);
    REQUIRE(result != CommandExecutor::COMMAND_ERROR);
    // This ensures that the tests do not block indefinitely
    usleep(500);
    REQUIRE(limitIdx < 500);
  }
  REQUIRE(result == returnvalue::FAILED);
  REQUIRE(cmdExecutor.getLastError() == 1);
}

#endif
