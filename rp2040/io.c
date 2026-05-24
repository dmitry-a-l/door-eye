#include "io.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdarg.h>

void send(const char *s) {
    printf("%s\n", s);
    stdio_flush();
}

void sendf(const char *fmt, ...) {
    char buf[64];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("%s\n", buf);
    stdio_flush();
}
