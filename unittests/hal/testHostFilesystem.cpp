#include <catch2/catch_test_macros.hpp>

#include "fsfw_hal/host/HostFilesystem.h"
#include <filesystem>

TEST_CASE("Host Filesystem", "[hal][host]") {
  namespace fs = std::filesystem;
  auto hostFs = HostFilesystem();
  auto tmpDir = fs::temp_directory_path();

  SECTION("Create file") {
    fs::path file = tmpDir / "hello.txt";
    FilesystemParams params(file.c_str());
    REQUIRE(hostFs.createFile(params) == result::OK);
    REQUIRE(fs::exists(file));
    REQUIRE_NOTHROW(fs::remove(file));
  }

  SECTION("Remove File") {
    fs::path file = tmpDir / "hello.txt";
    FilesystemParams params(file.c_str());
    REQUIRE(hostFs.createFile(params) == result::OK);
    REQUIRE(fs::exists(file));
    REQUIRE(hostFs.removeFile(file.c_str()) == result::OK);
    REQUIRE(not fs::exists(file));
  }

}