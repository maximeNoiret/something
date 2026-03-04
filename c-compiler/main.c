#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define output stdout


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
  unsigned line_count = 1;
  unsigned col = 0;
  unsigned wrd = 0;
  for (int in = fgetc(file); in != EOF && col < 32; in = fgetc(file)) {
    if (in == '#') {
      while (in != '\n' && in != EOF) in = fgetc(file);
    }
    if (in == ';' || in == ':') {
      if (in == ':') {
        fprintf(output, "\n%s:\n", buffer[0]);
      }
      else if (wrd == 0 && strcasecmp(buffer[0], "exit") == 0) {
        fputs("\nori $v0, $zero, 10\nsyscall\n", output);
      }
      else if (wrd == 1 && strcasecmp(buffer[0], "goto") == 0) {
        fprintf(output, "j %s\n", buffer[1]);
      }
      else if (wrd == 2 && strcasecmp(buffer[0], "if") == 0) {
        // TODO: conditional tri-branching
        puts("if statement");
      }
      else if (wrd == 3 && buffer[2][0] == '=') {
        // TODO: variable creation
        puts("var def statement");
      }
      else if (wrd == 2 && buffer[1][0] == '=') {
        // TODO: variable value replace/set
        puts("var set statement");
      }
      else {
        fprintf(stderr, "Something went wrong: Unrecognizable Syntax.\n\tLine: %d\n\t", line_count);
        for (unsigned word = 0; word <= wrd; ++word) {
          fprintf(stderr, "%s ", buffer[word]);
        }
        fputc('\n', stderr);
        return -1;
      }
      for (unsigned word = 0; word <= wrd; ++word) {
        memset(buffer[word], 0, 32);
      }
      col = 0;
      wrd = 0;
      continue;
    }
    switch (in) {
      case '\n':
        ++line_count;
      case ' ':
      case '\t':
        if (col > 0) {
          ++wrd;
          col = 0;
        }
        break;
      default:
        buffer[wrd][col++] = (char)in;
    }
  }
  fclose(file);
  return 0;
}