#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include "fsfw_hal/host/HostFilesystem.h"

TEST_CASE("Host Filesystem", "[hal][host]") {
  namespace fs = std::filesystem;
  auto hostFs = HostFilesystem();
  auto tmpDir = fs::temp_directory_path();
  fs::path file0 = tmpDir / "hello.txt";
  fs::path file1 = tmpDir / "hello.txt";
  REQUIRE_NOTHROW(fs::remove(file0));
  REQUIRE_NOTHROW(fs::remove(file1));

  SECTION("Create file") {
    fs::path file = tmpDir / "hello.txt";
    FilesystemParams params(file.c_str());
    REQUIRE(hostFs.createFile(params) == result::OK);
    CHECK(fs::is_regular_file(file));
    REQUIRE(fs::exists(file));
    REQUIRE_NOTHROW(fs::remove(file));
  }

  SECTION("Remove File") {
    fs::path file = tmpDir / "hello.txt";
    FilesystemParams params(file.c_str());
    REQUIRE(hostFs.createFile(params) == result::OK);
    CHECK(fs::is_regular_file(file));
    REQUIRE(fs::exists(file));
    REQUIRE(hostFs.removeFile(file.c_str()) == result::OK);
    REQUIRE(not fs::exists(file));
  }

  SECTION("Create Directory") {
    fs::path dirPath = tmpDir / "test_dir";
    FilesystemParams params(dirPath.c_str());
    REQUIRE(hostFs.createDirectory(params) == result::OK);
    CHECK(fs::is_directory(dirPath));
    REQUIRE(fs::exists(dirPath));
    REQUIRE_NOTHROW(fs::remove(dirPath));
  }

  SECTION("Remove Directory") {
    fs::path dirPath = tmpDir / "test_dir";
    FilesystemParams params(dirPath.c_str());
    REQUIRE(hostFs.createDirectory(params) == result::OK);
    REQUIRE(fs::exists(dirPath));
    REQUIRE(hostFs.removeDirectory(params) == result::OK);
    REQUIRE(not fs::exists(dirPath));
  }

  SECTION("Rename File") {
    fs::path file = tmpDir / "hello.txt";
    fs::path newFile = tmpDir / "hello2.txt";
    FilesystemParams params(file.c_str());
    REQUIRE(hostFs.createFile(params) == result::OK);
    CHECK(fs::is_regular_file(file));
    REQUIRE(fs::exists(file));
    REQUIRE(hostFs.rename(file.c_str(), newFile.c_str()) == result::OK);
    REQUIRE_NOTHROW(fs::remove(newFile));
  }
}