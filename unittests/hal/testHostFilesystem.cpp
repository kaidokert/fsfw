#include <catch2/catch_test_macros.hpp>

#include "fsfw_hal/host/HostFilesystem.h"

TEST_CASE("Host Filesystem", "[hal][host]") { auto hostFs = HostFilesystem(); }