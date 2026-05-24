#pragma once

#include <stdbool.h>
#include <stdint.h>

#define VPIN100  100
#define VPIN200  200

bool get_vpin100(void);
bool get_vpin200(void);
void poll_vpins(void);
