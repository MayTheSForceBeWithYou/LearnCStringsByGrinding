/* greet.h -- s05-e06-fortify-abort: a greeting in a fixed buffer. */
#ifndef GREET_H
#define GREET_H

#define GREET_SIZE 16

/* Builds "Hello, <name>!" in a buffer of GREET_SIZE bytes and prints it.
 * The buffer is this function's own; `name` is borrowed. */
void greet_print(const char *name);

#endif
