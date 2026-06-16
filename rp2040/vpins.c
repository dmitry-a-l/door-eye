#include "vpins.h"
#include "pins.h"
#include "pico/time.h"
#include "hardware/gpio.h"

/* ── vpin100: PIN26, прямой, 60 сек ──────────────────────────── */

#define VPIN100_DURATION_MS   60000

static uint64_t vpin100_expires_us = 0;

/* ── vpin200: PIN28, инверсия, 5 сек + 5 сек cooldown ───────── */

#define VPIN200_DURATION_MS   1000
#define VPIN200_COOLDOWN_MS   2000

static uint64_t vpin200_expires_us = 0;
static uint64_t pin28_cooldown_us  = 0;

/* ── getters ──────────────────────────────────────────────────── */

uint32_t get_vpin100_secs(void) {
    if (vpin100_expires_us == 0) return 0;
    uint64_t now = time_us_64();
    if (now >= vpin100_expires_us) {
        vpin100_expires_us = 0;
        return 0;
    }
    return (uint32_t)((vpin100_expires_us - now + 999999) / 1000000);
}

bool get_vpin200(void) {
    if (vpin200_expires_us == 0) return false;
    if (time_us_64() >= vpin200_expires_us) {
        vpin200_expires_us = 0;
        return false;
    }
    return true;
}

/* ── poll ─────────────────────────────────────────────────────── */

static void poll_vpin100(void) {
    if (gpio_get(PIN26))
        vpin100_expires_us = time_us_64() + (uint64_t)VPIN100_DURATION_MS * 1000;
}

static void poll_vpin200(void) {
    if (time_us_64() < pin28_cooldown_us) return;

    if (!gpio_get(PIN28)) {
        uint64_t now       = time_us_64();
        vpin200_expires_us = now + (uint64_t)VPIN200_DURATION_MS * 1000;
        pin28_cooldown_us  = now + (uint64_t)(VPIN200_DURATION_MS
                                            + VPIN200_COOLDOWN_MS) * 1000;
    }
}

void poll_vpins(void) {
    poll_vpin100();
    poll_vpin200();
}
