#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


// NOTE: the compiler will not have functions until I actually implement that in the compiler. This is so that bootstrapping is easier.
// Apologies in advance for the (probably) messy code that will ensue from this choice.
int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Incorrect argument count.\n\tCorrect usage: %s [code.something]\n", argv[0]);
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    fputs(strerror(errno), stderr);
    fputc('\n', stderr);
    return errno;
  }

  char buffer[5][32];  // 5 words of 32 characters, right?
  for (unsigned word = 0; word < 5; ++word) {
    memset(buffer[word], 0, 32);
  }
  unsigned col = 0;
  unsigned wrd = 0;
  for (int in = fgetc(file); in != EOF && col < 32; in = fgetc(file)) {
    if (in == ';') {
      // TODO: treat instruction
      for (unsigned word = 0; word <= wrd; ++word) {
        printf("%s", buffer[word]);
        if (word != wrd) putc(',', stdout);
        memset(buffer[word], 0, 32);
      }
      printf("\n");
      col = 0;
      wrd = 0;
      continue;
    }
    switch (in) {
      case ' ':
      case '\t':
      case '\n':
        if (col > 0) {
          ++wrd;
          col = 0;
        }
        break;
      default:
        buffer[wrd][col++] = (char)in;
    }
  }
  return 0;
}