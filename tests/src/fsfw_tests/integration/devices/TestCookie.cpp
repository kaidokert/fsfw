#include "TestCookie.h"

TestCookie::TestCookie(address_t address, size_t replyMaxLen)
    : address(address), replyMaxLen(replyMaxLen) {}

TestCookie::~TestCookie() {}

address_t TestCookie::getAddress() const { return address; }

size_t TestCookie::getReplyMaxLen() const { return replyMaxLen; }
