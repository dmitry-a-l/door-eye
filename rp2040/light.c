#include "light.h"
#include "autolock.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define PIN_LED_WHITE         2
#define PIN_LED_RED           3
#define PIN_SENSOR_LOCK_OPEN  10

#define FLASH_PERIOD_MS       2000   /* the flash count is "per 2 seconds" */
#define FLASH_ON_MS           100    /* nominal on-time of one short flash */

/* Red-LED autolock countdown: number of short flashes per 2 s window, growing as
 * the auto-close deadline approaches. */
static int autolock_flashes(uint32_t remaining_ms) {
    uint32_t s = remaining_ms / 1000;
    if (s <  10) return 20;   /* < 10 s  */
    if (s <= 30) return 10;   /*   30 s  */
    if (s <= 60) return 5;    /*    1 min */
    if (s <= 120) return 4;   /*    2 min */
    if (s <= 180) return 3;   /*    3 min */
    if (s <= 240) return 2;   /*    4 min */
    return 1;                 /*    5 min (and above) */
}

void light_leds(void) {
    bool lock_open = gpio_get(PIN_SENSOR_LOCK_OPEN);

    /* white LED: on while the lock is closed */
    gpio_put(PIN_LED_WHITE, lock_open ? 0 : 1);

    /* red LED: off while the lock is closed */
    if (!lock_open) {
        gpio_put(PIN_LED_RED, 0);
        return;
    }

    uint32_t remaining = autolock_remaining_ms();

    /* lock open, no autolock pending -> solid red */
    if (remaining == 0) {
        gpio_put(PIN_LED_RED, 1);
        return;
    }

    /* lock open with a pending autolock -> flash the countdown */
    int      flashes = autolock_flashes(remaining);
    uint32_t now     = to_ms_since_boot(get_absolute_time());
    uint32_t slot    = FLASH_PERIOD_MS / flashes;
    uint32_t on_ms   = slot / 2 < FLASH_ON_MS ? slot / 2 : FLASH_ON_MS;
    bool     on      = (now % FLASH_PERIOD_MS) % slot < on_ms;

    gpio_put(PIN_LED_RED, on ? 1 : 0);
}
