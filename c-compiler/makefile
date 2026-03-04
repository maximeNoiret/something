COMPILER = gcc
CFLAGS = -Wall -Werror -g -fsanitize=address,undefined
COMPILE_OBJECT = $(COMPILER) $(CFLAGS) -c $< -o $@

main: main.o
	$(COMPILER) $(CFLAGS) -o $@ $^

main.o: main.c
	$(COMPILE_OBJECT)