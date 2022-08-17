#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

#include "fsfw_hal/host/HostFilesystem.h"

using namespace std;

TEST_CASE("Host Filesystem", "[hal][host]") {
  namespace fs = filesystem;
  auto hostFs = HostFilesystem();
  auto tmpDir = fs::temp_directory_path();
  fs::path file0 = tmpDir / "hello.txt";
  fs::path file1 = tmpDir / "hello2.txt";
  fs::path dir0 = tmpDir / "test_dir";
  REQUIRE_NOTHROW(fs::remove(file0));
  REQUIRE_NOTHROW(fs::remove(file1));
  REQUIRE_NOTHROW(fs::remove(dir0));

  SECTION("Create file") {
    FilesystemParams params(file0.c_str());
    REQUIRE(hostFs.createFile(params) == result::OK);
    CHECK(fs::is_regular_file(file0));
    REQUIRE(fs::exists(file0));
    REQUIRE_NOTHROW(fs::remove(file0));
  }

  SECTION("Remove File") {
    FilesystemParams params(file0.c_str());
    REQUIRE(hostFs.createFile(params) == result::OK);
    CHECK(fs::is_regular_file(file0));
    REQUIRE(fs::exists(file0));
    REQUIRE(hostFs.removeFile(file0.c_str()) == result::OK);
    REQUIRE(not fs::exists(file0));
  }

  SECTION("Create Directory") {
    FilesystemParams params(dir0.c_str());
    REQUIRE(hostFs.createDirectory(params) == result::OK);
    CHECK(fs::is_directory(dir0));
    REQUIRE(fs::exists(dir0));
    REQUIRE_NOTHROW(fs::remove(dir0));
  }

  SECTION("Remove Directory") {
    FilesystemParams params(dir0.c_str());
    REQUIRE(hostFs.createDirectory(params) == result::OK);
    REQUIRE(fs::exists(dir0));
    REQUIRE(hostFs.removeDirectory(params) == result::OK);
    REQUIRE(not fs::exists(dir0));
  }

  SECTION("Rename File") {
    FilesystemParams params(file0.c_str());
    REQUIRE(hostFs.createFile(params) == result::OK);
    CHECK(fs::is_regular_file(file0));
    REQUIRE(fs::exists(file0));
    REQUIRE(hostFs.rename(file0.c_str(), file1.c_str()) == result::OK);
    REQUIRE_NOTHROW(fs::remove(file1));
  }

  SECTION("Write To File") {
    std::string data = "hello world!";
    FileOpParams params(file0.c_str(), data.size());
    REQUIRE(hostFs.createFile(params.fsParams) == result::OK);
    CHECK(fs::is_regular_file(file0));
    REQUIRE(fs::exists(file0));
    CHECK(hostFs.writeToFile(params, reinterpret_cast<const uint8_t*>(data.c_str())) ==
          HasReturnvaluesIF::RETURN_OK);
    CHECK(fs::file_size(file0) == data.size());
    ifstream ifile(file0);
    char readBuf[524]{};
    ifile.read(readBuf, sizeof(readBuf));
    std::string readBackString(readBuf);
    CHECK(data == readBackString);
    REQUIRE_NOTHROW(fs::remove(file1));
  }

  SECTION("Read From File") {
    std::string data = "hello world!";
    FileOpParams params(file0.c_str(), data.size());
    REQUIRE(hostFs.createFile(params.fsParams) == result::OK);
    CHECK(fs::is_regular_file(file0));
    ofstream of(file0);
    of.write(data.c_str(), static_cast<unsigned int>(data.size()));
    of.close();
    CHECK(fs::file_size(file0) == data.size());
    REQUIRE(fs::exists(file0));
    std::array<uint8_t, 256> readBuf{};
    uint8_t* readPtr = readBuf.data();
    size_t readSize = 0;
    CHECK(hostFs.readFromFile(params, &readPtr, readSize, readBuf.size()) ==
          HasReturnvaluesIF::RETURN_OK);
    std::string readBackString(reinterpret_cast<const char*>(readBuf.data()));
    CHECK(readSize == data.size());
    CHECK(data == readBackString);
    REQUIRE_NOTHROW(fs::remove(file1));
  }

  REQUIRE_NOTHROW(fs::remove(file0));
  REQUIRE_NOTHROW(fs::remove(file1));
  REQUIRE_NOTHROW(fs::remove(dir0));
}