#include <catch2/catch_test_macros.hpp>

#include "fsfw/tmtcservices/PusServiceBase.h"

TEST_CASE("Pus Service Base", "[pus-service-base]") { auto psb = PusServiceBase() }