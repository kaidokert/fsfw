#include "fsfw_tests/internal/globalfunctions/TestArrayPrinter.h"

void arrayprinter::testArrayPrinter() {
  {
    const std::array<uint8_t, 5> testDataSmall = {0x01, 0x02, 0x03, 0x04, 0x05};
    arrayprinter::print(testDataSmall.data(), testDataSmall.size());
    arrayprinter::print(testDataSmall.data(), testDataSmall.size(), OutputType::DEC);
    arrayprinter::print(testDataSmall.data(), testDataSmall.size(), OutputType::BIN);
  }

  {
    std::array<uint8_t, 16> testDataMed;
    for (size_t idx = 0; idx < testDataMed.size(); idx++) {
      testDataMed[idx] = testDataMed.size() - idx;
    }
    arrayprinter::print(testDataMed.data(), testDataMed.size());
    arrayprinter::print(testDataMed.data(), testDataMed.size(), OutputType::DEC, 8);
  }

  {
    std::array<uint8_t, 32> testDataLarge;
    for (size_t idx = 0; idx < testDataLarge.size(); idx++) {
      testDataLarge[idx] = idx;
    }
    arrayprinter::print(testDataLarge.data(), testDataLarge.size());
    arrayprinter::print(testDataLarge.data(), testDataLarge.size(), OutputType::DEC);
  }
}
