#include <catch2/catch_test_macros.hpp>

#include "fsfw/serialize/SerializeIF.h"
#include "mocks/FilesystemMock.h"

using namespace std;

TEST_CASE("Filesystem Mock", "[mocks]") {
  auto fsMock = FilesystemMock();

  SECTION("Create File") {
    FilesystemParams params("hello.txt");
    CHECK(fsMock.createFile(params) == returnvalue::OK);
    auto iter = fsMock.fileMap.find("hello.txt");
    REQUIRE(iter != fsMock.fileMap.end());
    FilesystemMock::FileInfo &stats = iter->second;
    CHECK(stats.fileSegQueue.empty());
    CHECK(stats.fileRaw.empty());
  }

  SECTION("Write to File") {
    std::string testData = "test data";
    FileOpParams params("hello.txt", testData.size());
    CHECK(fsMock.writeToFile(params, reinterpret_cast<const uint8_t *>(testData.data())) ==
          returnvalue::OK);
    auto iter = fsMock.fileMap.find("hello.txt");
    REQUIRE(iter != fsMock.fileMap.end());
    FilesystemMock::FileInfo &stats = iter->second;
    CHECK(not stats.fileSegQueue.empty());
    CHECK(not stats.fileRaw.empty());
    auto &segment = stats.fileSegQueue.back();
    CHECK(segment.offset == 0);
    CHECK(std::string(reinterpret_cast<const char *>(segment.data.data()), segment.data.size()) ==
          testData);
    CHECK(std::string(reinterpret_cast<const char *>(stats.fileRaw.data()), segment.data.size()) ==
          testData);
  }

  SECTION("Create Directory") {
    FilesystemParams params("hello");
    CHECK(fsMock.createDirectory(params) == returnvalue::OK);
    REQUIRE(not fsMock.dirMap.empty());
    auto iter = fsMock.dirMap.find("hello");
    REQUIRE(iter != fsMock.dirMap.end());
    auto &dirInfo = iter->second;
    CHECK(dirInfo.createCallCount == 1);
    CHECK(dirInfo.delCallCount == 0);
  }
}