#include "printChar.h"

#include <cstdio>

void printChar(const char* character, bool errStream) {
  if (errStream) {
    std::putc(*character, stderr);
    return;
  }
  std::putc(*character, stdout);
}
