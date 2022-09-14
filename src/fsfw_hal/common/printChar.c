#include <stdbool.h>
#include <stdio.h>

void __attribute__((weak)) printChar(const char* character, bool errStream) {
  if (errStream) {
    fprintf(stderr, "%c", *character);
  } else {
    printf("%c", *character);
  }
}
