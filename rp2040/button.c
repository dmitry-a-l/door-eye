#include "button.h"
#include "lock.h"
#include "autolock.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define PIN_BUTTON             8    /* action button (active high, pulldown) */
#define PIN_SENSOR_DOOR_CLOSE  13   /* 1 - door closed, 0 - door open */
#define PIN_SENSOR_LOCK_OPEN   10   /* 1 - lock open,   0 - lock closed */
#define PIN_BUZZER             5

#define HOLD_MS                1000

#define AUTOLOCK_5MIN_MS       (5 * 60 * 1000)

static void beep(uint32_t on_ms) {
    gpio_put(PIN_BUZZER, 1);
    sleep_ms(on_ms);
    gpio_put(PIN_BUZZER, 0);
}

/* Action button behaviour, decided from the lock state at the moment of pressing:
 *
 *   Lock CLOSED at press:
 *     - open the lock immediately on press, regardless of press length (the open
 *       itself makes no beep).
 *     - if held to 2 s / 5 s it additionally arms the sticky 2 / 5 min hold and
 *       beeps 1 s / twice as confirmation.
 *
 *   Lock OPEN at press (door closed):
 *     - short press (released before 2 s) -> close the lock (toggle).
 *     - held 2 s / 5 s -> arm the sticky 2 / 5 min hold, beep 1 s / twice as
 *       confirmation.
 *
 * The hold is sticky: autolock_set() makes the window survive the door being
 * opened and closed (see autolock.c), so it locks only after the full 2 / 5 min. */
void button_poll(void) {
    static bool     pressed_prev       = false;
    static uint32_t press_start        = 0;
    static bool     fired_long_press   = false;
    static bool     lock_open_at_press = false;

    bool     pressed = gpio_get(PIN_BUTTON);
    uint32_t now     = to_ms_since_boot(get_absolute_time());

    /* press edge */
    if (pressed && !pressed_prev) {
        press_start        = now;
        fired_long_press   = false;
        lock_open_at_press = gpio_get(PIN_SENSOR_LOCK_OPEN);

        /* lock closed -> open immediately, regardless of press length */
        if (!lock_open_at_press)
            open_lock(true);
    }

    /* held — arm the sticky hold at the thresholds; only beep when the gesture
     * started with the lock already open (opening a closed lock stays silent) */
    if (pressed) {
        uint32_t held = now - press_start;

        if (!fired_long_press && held >= HOLD_MS) {
            fired_long_press = true;
            autolock_set(AUTOLOCK_5MIN_MS);
            beep(1000);
        }
    }

    /* release edge — short press on an already-open lock closes it (toggle) */
    if (!pressed && pressed_prev) {
        if (!fired_long_press && lock_open_at_press &&
            gpio_get(PIN_SENSOR_DOOR_CLOSE)) {
            close_lock(true);
        }
    }

    pressed_prev = pressed;
}
