#include "autolock.h"
#include "lock.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define PIN_SENSOR_LOCK_OPEN   10
#define PIN_SENSOR_DOOR_CLOSE  13
#define PIN_BUZZER             5

#define AUTOLOCK_AFTER_DOOR_CLOSE_MS   10000
#define AUTOLOCK_AFTER_LOCK_OPEN_MS    30000
#define AUTOLOCK_WARN_BEFORE_MS        3000

static void buzzer_warn(void) {
    for (int i = 0; i < 3; i++) {
        gpio_put(PIN_BUZZER, 1);
        sleep_ms(100);
        gpio_put(PIN_BUZZER, 0);
        sleep_ms(100);
    }
}

void autolock_poll(void) {
    bool door_closed = gpio_get(PIN_SENSOR_DOOR_CLOSE);
    bool lock_open   = gpio_get(PIN_SENSOR_LOCK_OPEN);
    uint32_t now     = to_ms_since_boot(get_absolute_time());

    static bool initialized  = false;
    static bool last_door_closed = false;
    static bool last_lock_open   = false;
    static uint32_t door_closed_at = 0;
    static uint32_t lock_opened_at = 0;
    static bool warned_door = false;
    static bool warned_lock = false;

    if (!initialized) {
        last_door_closed = door_closed;
        last_lock_open   = lock_open;
        initialized      = true;
        return;
    }

    /* дверь только что закрылась */
    if (door_closed && !last_door_closed) {
        door_closed_at = now;
        warned_door    = false;
    }

    /* замок открылся при закрытой двери */
    if (door_closed && lock_open && !last_lock_open) {
        lock_opened_at = now;
        warned_lock    = false;
    }

    /* дверь открылась — сбросить таймеры */
    if (!door_closed) {
        door_closed_at = 0;
        lock_opened_at = 0;
        warned_door    = false;
        warned_lock    = false;
    }

    /* замок закрылся — сбросить таймер открытия */
    if (!lock_open) {
        lock_opened_at = 0;
        warned_lock    = false;
    }

    /* предупреждение перед door autolock */
    if (door_closed_at > 0 && lock_open) {
        uint32_t elapsed = now - door_closed_at;
        if (!warned_door &&
            elapsed >= (AUTOLOCK_AFTER_DOOR_CLOSE_MS - AUTOLOCK_WARN_BEFORE_MS)) {
            buzzer_warn();
            warned_door = true;
        }
        if (elapsed >= AUTOLOCK_AFTER_DOOR_CLOSE_MS) {
            door_closed_at = 0;
            close_lock(true);
        }
    }

    /* предупреждение перед lock autolock */
    if (lock_opened_at > 0) {
        uint32_t elapsed = now - lock_opened_at;
        if (!warned_lock &&
            elapsed >= (AUTOLOCK_AFTER_LOCK_OPEN_MS - AUTOLOCK_WARN_BEFORE_MS)) {
            buzzer_warn();
            warned_lock = true;
        }
        if (elapsed >= AUTOLOCK_AFTER_LOCK_OPEN_MS) {
            lock_opened_at = 0;
            close_lock(true);
        }
    }

    last_door_closed = door_closed;
    last_lock_open   = lock_open;
}
