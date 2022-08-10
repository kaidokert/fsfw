#include "StringLv.h"

cfdp::StringLv::StringLv(const std::string& fileName)
    : Lv(reinterpret_cast<const uint8_t*>(fileName.data()), fileName.size()) {}

cfdp::StringLv::StringLv(const char* filename, size_t len)
    : Lv(reinterpret_cast<const uint8_t*>(filename), len) {}

cfdp::StringLv::StringLv() : Lv() {}
