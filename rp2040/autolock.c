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

static uint32_t autolock_at = 0;   /* абсолютное время автозакрытия, 0 = не запланировано */
static bool     warned      = false;

static void buzzer_warn(void) {
    for (int i = 0; i < 3; i++) {
        gpio_put(PIN_BUZZER, 1);
        sleep_ms(100);
        gpio_put(PIN_BUZZER, 0);
        sleep_ms(100);
    }
}

static void schedule_autolock(uint32_t now, uint32_t delay_ms) {
    autolock_at = now + delay_ms;
    warned      = false;
}

static void cancel_autolock(void) {
    autolock_at = 0;
    warned      = false;
}

void autolock_poll(void) {
    bool door_closed = gpio_get(PIN_SENSOR_DOOR_CLOSE);
    bool lock_open   = gpio_get(PIN_SENSOR_LOCK_OPEN);
    uint32_t now     = to_ms_since_boot(get_absolute_time());

    static bool last_door_closed = false;
    static bool last_lock_open   = false;
    static bool initialized      = false;

    /* первый тик — инициализация без срабатывания */
    if (!initialized) {
        last_door_closed = door_closed;
        last_lock_open   = lock_open;
        initialized      = true;
        return;
    }

    /* ── обновление таймеров ──────────────────────────────────── */

    /* дверь открылась — сбросить всё */
    if (!door_closed && last_door_closed) {
        cancel_autolock();
    }

    /* дверь закрылась — закрыть замок через 10 сек */
    if (door_closed && !last_door_closed) {
        schedule_autolock(now, AUTOLOCK_AFTER_DOOR_CLOSE_MS);
    }

    /* замок открылся при закрытой двери — закрыть через 30 сек */
    if (lock_open && !last_lock_open && door_closed) {
        schedule_autolock(now, AUTOLOCK_AFTER_LOCK_OPEN_MS);
    }

    last_door_closed = door_closed;
    last_lock_open   = lock_open;

    /* ── выполнение таймера ───────────────────────────────────── */

    if (autolock_at == 0) return;

    /* предупреждение за 3 секунды */
    if (!warned && now >= autolock_at - AUTOLOCK_WARN_BEFORE_MS) {
        buzzer_warn();
        warned = true;
    }

    /* время вышло — закрыть замок */
    if (now >= autolock_at) {
        cancel_autolock();
        close_lock(true);
    }
}
