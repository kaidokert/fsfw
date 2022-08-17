#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

#include "fsfw/serialize/SerializeIF.h"
#include "fsfw_hal/host/HostFilesystem.h"

using namespace std;

TEST_CASE("Host Filesystem", "[hal][host]") {
  namespace fs = filesystem;
  auto hostFs = HostFilesystem();
  auto tmpDir = fs::temp_directory_path();
  fs::path file0 = tmpDir / "hello.txt";
  fs::path file1 = tmpDir / "hello2.txt";
  fs::path dir0 = tmpDir / "test_dir";
  fs::path fileInDir0 = dir0 / "hello.txt";
  fs::path dirWithParent = dir0 / "test_dir";

  REQUIRE_NOTHROW(fs::remove(file0));
  REQUIRE_NOTHROW(fs::remove(file1));
  REQUIRE_NOTHROW(fs::remove_all(dir0));

  SECTION("Create file") {
    FilesystemParams params(file0.c_str());
    REQUIRE(hostFs.createFile(params) == result::OK);
    CHECK(fs::is_regular_file(file0));
    REQUIRE(fs::exists(file0));
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
  }

  SECTION("Invalid Input does not crash") {
    FileOpParams params(nullptr, 10);
    REQUIRE(hostFs.createFile(params.fsParams) != result::OK);
    REQUIRE(hostFs.createDirectory(params.fsParams) != result::OK);
    REQUIRE(hostFs.createFile(params.fsParams) != result::OK);
    REQUIRE(hostFs.removeDirectory(params.fsParams) != result::OK);
    REQUIRE(hostFs.removeFile(nullptr) != result::OK);
    REQUIRE(hostFs.rename(nullptr, nullptr) != result::OK);
    REQUIRE(hostFs.writeToFile(params, nullptr) != result::OK);
    size_t readLen = 0;
    REQUIRE(hostFs.readFromFile(params, nullptr, readLen, 20) != result::OK);
  }

  SECTION("Create File but already exists") {
    FilesystemParams params(file0.c_str());
    REQUIRE(hostFs.createFile(params) == result::OK);
    REQUIRE(hostFs.createFile(params) == HasFileSystemIF::FILE_ALREADY_EXISTS);
  }

  SECTION("Remove File but does not exist") {
    REQUIRE(hostFs.removeFile(file0.c_str()) == HasFileSystemIF::FILE_DOES_NOT_EXIST);
  }

  SECTION("Create Directory but already exists") {
    FileOpParams params(file0.c_str(), 12);
    REQUIRE(hostFs.createDirectory(params.fsParams) == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(hostFs.createDirectory(params.fsParams) == HasFileSystemIF::DIRECTORY_ALREADY_EXISTS);
  }

  SECTION("Remove Directory but does not exist") {
    FilesystemParams params(dir0.c_str());
    REQUIRE(hostFs.removeDirectory(params) == HasFileSystemIF::DIRECTORY_DOES_NOT_EXIST);
  }

  SECTION("Remove Directory but is file") {
    ofstream of(file0);
    FilesystemParams params(file0.c_str());
    REQUIRE(hostFs.removeDirectory(params) == HasFileSystemIF::NOT_A_DIRECTORY);
  }

  SECTION("Read from file but does not exist") {
    std::string data = "hello world!";
    FileOpParams params(file0.c_str(), data.size());
    std::array<uint8_t, 10> readBuf{};
    uint8_t* readPtr = readBuf.data();
    size_t readSize = 0;
    CHECK(hostFs.readFromFile(params, &readPtr, readSize, readBuf.size()) ==
          HasFileSystemIF::FILE_DOES_NOT_EXIST);
  }

  SECTION("Write to file but does not exist") {
    std::string data = "hello world!";
    FileOpParams params(file0.c_str(), data.size());
    CHECK(hostFs.writeToFile(params, reinterpret_cast<const uint8_t*>(data.c_str())) ==
          HasFileSystemIF::FILE_DOES_NOT_EXIST);
  }

  SECTION("Remove recursively") {
    fs::create_directory(dir0.c_str());
    ofstream of(fileInDir0);
    CHECK(fs::is_directory(dir0));
    CHECK(fs::is_regular_file(fileInDir0));
    REQUIRE(hostFs.removeDirectory(FilesystemParams(dir0.c_str()), true) == result::OK);
    CHECK(not fs::is_directory(dir0));
    CHECK(not fs::is_regular_file(fileInDir0));
  }

  SECTION("Non-Recursive Removal Fails") {
    fs::create_directory(dir0.c_str());
    ofstream of(fileInDir0);
    CHECK(fs::is_directory(dir0));
    CHECK(fs::is_regular_file(fileInDir0));
    REQUIRE(hostFs.removeDirectory(FilesystemParams(dir0.c_str())) ==
            HasFileSystemIF::DIRECTORY_NOT_EMPTY);
  }

  SECTION("Create directory with parent directory") {
    CHECK(hostFs.createDirectory(FilesystemParams(dirWithParent.c_str()), true) == result::OK);
    CHECK(fs::is_directory(dir0));
    CHECK(fs::is_directory(dirWithParent));
  }

  SECTION("Read but provided buffer too small") {
    std::string data = "hello world!";
    FileOpParams params(file0.c_str(), data.size());
    ofstream of(file0);
    of.write(data.c_str(), static_cast<unsigned int>(data.size()));
    of.close();
    CHECK(fs::file_size(file0) == data.size());
    REQUIRE(fs::exists(file0));
    std::array<uint8_t, 15> readBuf{};
    uint8_t* readPtr = readBuf.data();
    size_t readSize = 0;
    CHECK(hostFs.readFromFile(params, &readPtr, readSize, 5) == SerializeIF::BUFFER_TOO_SHORT);
    readSize = 10;
    CHECK(hostFs.readFromFile(params, &readPtr, readSize, readBuf.size()) ==
          SerializeIF::BUFFER_TOO_SHORT);
  }

  REQUIRE_NOTHROW(fs::remove(file0));
  REQUIRE_NOTHROW(fs::remove(file1));
  REQUIRE_NOTHROW(fs::remove_all(dir0));
}