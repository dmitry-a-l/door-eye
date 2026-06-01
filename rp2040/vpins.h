#pragma once

#include <stdbool.h>
#include <stdint.h>

#define VPIN100  100
#define VPIN200  200

uint32_t get_vpin100_secs(void);
bool get_vpin200(void);
void poll_vpins(void);
