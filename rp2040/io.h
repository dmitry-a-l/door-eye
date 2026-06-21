#pragma once

#include <stdarg.h>

void set_reply_tag(const char *tag);
void send(const char *s);
void sendf(const char *fmt, ...);
