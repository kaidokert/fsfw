/**
 * @file 	CatchSource.cpp
 * @brief 	Source file to compile catch framework.
 * @details	All tests should be written in other files.
 * For eclipse console output, install ANSI Escape in Console
 * from the eclipse market place to get colored characters.
 */

#ifndef NO_UNIT_TEST_FRAMEWORK

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#if CUSTOM_UNITTEST_RUNNER == 0

extern int customSetup();

int main( int argc, char* argv[] ) {
    customSetup();

    // Catch internal function call
    int result = Catch::Session().run( argc, argv );

    // global clean-up
    return result;
}

#endif


#endif
