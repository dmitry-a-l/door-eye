#include "autolock.h"
#include "lock.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define PIN_SENSOR_LOCK_OPEN   10
#define PIN_SENSOR_DOOR_CLOSE  13

#define AUTOLOCK_AFTER_DOOR_CLOSE_MS   10000
#define AUTOLOCK_AFTER_LOCK_OPEN_MS    30000

static uint32_t door_closed_at  = 0;   /* когда дверь закрылась */
static uint32_t lock_opened_at  = 0;   /* когда замок открылся при закрытой двери */

static bool last_door_closed = false;
static bool last_lock_open   = false;

void autolock_poll(void) {
    bool door_closed = gpio_get(PIN_SENSOR_DOOR_CLOSE);
    bool lock_open   = gpio_get(PIN_SENSOR_LOCK_OPEN);
    uint32_t now     = to_ms_since_boot(get_absolute_time());

    /* дверь только что закрылась — запомнить время */
    if (door_closed && !last_door_closed) {
        door_closed_at = now;
    }

    /* замок открылся при закрытой двери — запомнить время */
    if (door_closed && lock_open && !last_lock_open) {
        lock_opened_at = now;
    }

    /* дверь открылась — сбросить таймеры */
    if (!door_closed) {
        door_closed_at = 0;
        lock_opened_at = 0;
    }

    /* замок закрылся — сбросить таймер открытия */
    if (!lock_open) {
        lock_opened_at = 0;
    }

    /* autolock после закрытия двери */
    if (door_closed_at > 0 && lock_open &&
        (now - door_closed_at) >= AUTOLOCK_AFTER_DOOR_CLOSE_MS) {
        door_closed_at = 0;
        close_lock(true);
    }

    /* autolock после открытия замка при закрытой двери */
    if (lock_opened_at > 0 &&
        (now - lock_opened_at) >= AUTOLOCK_AFTER_LOCK_OPEN_MS) {
        lock_opened_at = 0;
        close_lock(true);
    }

    last_door_closed = door_closed;
    last_lock_open   = lock_open;
}
