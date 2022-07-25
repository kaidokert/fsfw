/**
 * @file 	CatchRunner.cpp
 * @brief 	Source file to compile catch framework.
 * @details	All tests should be written in other files.
 * For eclipse console output, install ANSI Escape in Console
 * from the eclipse market place to get colored characters.
 */

#include "CatchRunner.h"

#define CATCH_CONFIG_COLOUR_WINDOWS

#include <catch2/catch_session.hpp>

extern int customSetup();
extern int customTeardown();

int main(int argc, char* argv[]) {
  customSetup();

  // Catch internal function call
  int result = Catch::Session().run(argc, argv);

  // global clean-up
  customTeardown();
  return result;
}
