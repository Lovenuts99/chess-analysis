#ifndef APSC143__PANIC_H
#define APSC143__PANIC_H

// Prints an error message and exits the program. Called with a format string
// and arguments in the same form as printf.
void panicf(const char *format, ...);

#endif
