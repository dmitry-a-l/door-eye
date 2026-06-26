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

static uint32_t autolock_at = 0;   /* absolute auto-close time, 0 = not scheduled */
static bool     warned      = false;
static bool     manual_hold = false;  /* sticky timer set via autolock_set() by a button
                                         long-press; it survives door open/close and the
                                         automatic 10s/30s rescheduling until it expires */

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
    manual_hold = false;
}

void autolock_set(uint32_t delay_ms) {
    schedule_autolock(to_ms_since_boot(get_absolute_time()), delay_ms);
    manual_hold = true;
}

uint32_t autolock_remaining_ms(void) {
    if (autolock_at == 0) return 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    return now >= autolock_at ? 0 : autolock_at - now;
}

void autolock_poll(void) {
    bool door_closed = gpio_get(PIN_SENSOR_DOOR_CLOSE);
    bool lock_open   = gpio_get(PIN_SENSOR_LOCK_OPEN);
    uint32_t now     = to_ms_since_boot(get_absolute_time());

    static bool last_door_closed = false;
    static bool last_lock_open   = false;
    static bool initialized      = false;

    /* first tick — initialize without firing */
    if (!initialized) {
        last_door_closed = door_closed;
        last_lock_open   = lock_open;
        initialized      = true;
        return;
    }

    /* ── timer updates ────────────────────────────────────────── */
    /* A manual hold (button long-press) ignores all automatic rescheduling so
     * its 2/5 min window survives the door being opened and closed. */

    /* door opened — reset everything */
    if (!door_closed && last_door_closed && !manual_hold) {
        cancel_autolock();
    }

    /* door closed — close the lock after 10 s */
    if (door_closed && !last_door_closed && !manual_hold) {
        schedule_autolock(now, AUTOLOCK_AFTER_DOOR_CLOSE_MS);
    }

    /* lock opened while door closed — close after 30 s */
    if (lock_open && !last_lock_open && door_closed && !manual_hold) {
        schedule_autolock(now, AUTOLOCK_AFTER_LOCK_OPEN_MS);
    }

    last_door_closed = door_closed;
    last_lock_open   = lock_open;

    /* ── timer execution ──────────────────────────────────────── */

    if (autolock_at == 0) return;

    /* nothing to close if the lock is already closed — cancel silently, no beep */
    if (!lock_open) {
        cancel_autolock();
        return;
    }

    /* warn 3 seconds before closing */
    if (!warned && now >= autolock_at - AUTOLOCK_WARN_BEFORE_MS) {
        buzzer_warn();
        warned = true;
    }

    /* time is up — close the lock */
    if (now >= autolock_at) {
        cancel_autolock();
        close_lock(true);
    }
}
