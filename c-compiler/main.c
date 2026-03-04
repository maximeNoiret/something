#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define output stdout
#define SYMB_LIM 64

typedef struct Symbol {
  char name[32];
  unsigned offset;
} Symbol;


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
  
  unsigned symbol_size = 0;
  // FIRST PASS for symbol_size. (yes, really. A WHOLE PASS just for one value. Awesome :3)
  for (int in = fgetc(file); in != EOF && col < 32; in = fgetc(file)) {
    if (in == '#') {
      while (in != '\n' && in != EOF) in = fgetc(file);
    }
    if (in == ';' || in == ':') {
      // only interested in var definition.
      if (wrd == 3 && buffer[2][0] == '=') {
        // TODO: change this with actual type size. For now everything is int.
        symbol_size += 4;
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

  for (unsigned word = 0; word < 5; ++word) {
    memset(buffer[word], 0, 32);
  }
  col = 0;
  wrd = 0;

  unsigned current_sp = symbol_size;
  unsigned symbol_counts = 0;
  Symbol symbols[SYMB_LIM];

  unsigned line_count = 1;
  // SECOND PASS for the REAL code. (this is so bad help)
  fseek(file, 0, 0);  // reset cursor
  fprintf(output, "\naddi $sp, $sp, -%d\n\n", symbol_size);
  for (int in = fgetc(file); in != EOF && col < 32; in = fgetc(file)) {
    if (in == '#') {
      while (in != '\n' && in != EOF) in = fgetc(file);
    }
    if (in == ';' || in == ':') {
      if (in == ':') {                                            // label
        fprintf(output, "\n%s:\n", buffer[0]);
      }
      else if (wrd == 0 && strcasecmp(buffer[0], "exit") == 0) {  // exit
        fputs("\nori $v0, $zero, 10\nsyscall\n", output);
      }
      else if (wrd == 1 && strcasecmp(buffer[0], "goto") == 0) {  // goto
        fprintf(output, "j %s\n", buffer[1]);
      }
      else if (wrd == 4 && strcasecmp(buffer[0], "if") == 0) {    // if statement
        // get offset
        unsigned offset = -1;
        for (unsigned i = 0; i < SYMB_LIM && symbols[i].name[0] != '\0'; ++i) {
          if (strcmp(symbols[i].name, buffer[1]) == 0) {
            offset = symbols[i].offset;
          }
        }
        if (offset == -1) {
          fprintf(stderr, "Something went wrong: Unknown variable.\n\tLine: %d\n\t'%s'\n", line_count, buffer[1]);
          return -1;
        }
        fprintf(output, "\nlw  $t0, %d($sp)\nbltz $t0, %s\nbgtz $t0, %s\nbeq $t0, $zero, %s\n", offset, buffer[2], buffer[4], buffer[3]);
      }
      else if (wrd == 3 && buffer[2][0] == '=') {                 // variable creation
        // get offset
        unsigned offset = -1;
        for (unsigned i = 0; i < SYMB_LIM && symbols[i].name[0] != '\0'; ++i) {
          if (strcmp(symbols[i].name, buffer[1]) == 0) {
            offset = symbols[i].offset;
          }
        }
        if (offset != -1) {
          fprintf(stderr, "Something went wrong: Variable exists.\n\tLine: %d\n\t'%s'\n", line_count, buffer[1]);
          return -1;
        }
        // TODO: differentiate variable types and sizes. FOR NOW everything is int (4 bytes).

        current_sp -= 4;
        // TODO: get rid of that icky strcpy ;-;
        strcpy(symbols[symbol_counts].name, buffer[1]);
        symbols[symbol_counts++].offset = current_sp;
        fprintf(output, "ori $t0, $zero, %s\nsw  $t0, %d($sp)\n", buffer[3], current_sp);
      }
      else if (wrd == 2 && buffer[1][0] == '=') {                 // variable set
        // get offset
        unsigned offset = -1;
        for (unsigned i = 0; i < SYMB_LIM && symbols[i].name[0] != '\0'; ++i) {
          if (strcmp(symbols[i].name, buffer[0]) == 0) {
            offset = symbols[i].offset;
          }
        }
        if (offset == -1) {
          fprintf(stderr, "Something went wrong: Unknown variable.\n\tLine: %d\n\t'%s'\n", line_count, buffer[0]);
          return -1;
        }
        fprintf(output, "ori $t0, $zero, %s\nsw  $t0, %d($sp)\n", buffer[2], offset);
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