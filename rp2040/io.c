#include "io.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* Request/response correlation tag. The host sends a command with a leading
 * tag (e.g. "@42 GET 8"); every reply to it is printed with the same tag
 * ("@42 1"). Unsolicited messages (READY, etc.) are sent without a tag and are
 * dropped by the host. */
static char reply_tag[16] = "";

void set_reply_tag(const char *tag) {
    if (tag) {
        strncpy(reply_tag, tag, sizeof(reply_tag) - 1);
        reply_tag[sizeof(reply_tag) - 1] = 0;
    } else {
        reply_tag[0] = 0;
    }
}

void send(const char *s) {
    if (reply_tag[0])
        printf("%s %s\n", reply_tag, s);
    else
        printf("%s\n", s);
    stdio_flush();
}

void sendf(const char *fmt, ...) {
    char buf[64];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    send(buf);
}
