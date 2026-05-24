#include "io.h"
#include "lock.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define PIN_MOTOR_CLOSE        0
#define PIN_MOTOR_OPEN         1
#define PIN_LED_WHITE          2
#define PIN_LED_RED            3

#define PIN_SENSOR_LOCK_OPEN   10   /* 1 - замок открыт,  0 - замок закрыт */
#define PIN_SENSOR_DOOR_CLOSE  13   /* 1 - дверь закрыта, 0 - дверь открыта */

#define POLL_INTERVAL_MS       100
#define SETTLE_MS              300
#define TIMEOUT_MS             3000

void set_lock_leds(void) {
    bool lock_open = gpio_get(PIN_SENSOR_LOCK_OPEN);
    gpio_put(PIN_LED_WHITE, lock_open ? 0 : 1);
    gpio_put(PIN_LED_RED,   lock_open ? 1 : 0);
}

void lock(void) {
    if (!gpio_get(PIN_SENSOR_DOOR_CLOSE)) { send("ERR - door is open");    return; }
    if (!gpio_get(PIN_SENSOR_LOCK_OPEN))  { send("ERR - lock already closed"); return; }

    gpio_put(PIN_MOTOR_CLOSE, 1);

    uint32_t elapsed = 0;
    while (elapsed < TIMEOUT_MS) {
        sleep_ms(POLL_INTERVAL_MS);
        elapsed += POLL_INTERVAL_MS;
        if (!gpio_get(PIN_SENSOR_LOCK_OPEN)) {
            sleep_ms(SETTLE_MS);
            break;
        }
    }

    gpio_put(PIN_MOTOR_CLOSE, 0);
    set_lock_leds();
    send("OK");
}

void unlock(void) {
    if (!gpio_get(PIN_SENSOR_DOOR_CLOSE)) { send("ERR - door is open");      return; }
    if (gpio_get(PIN_SENSOR_LOCK_OPEN))   { send("ERR - lock already open"); return; }

    gpio_put(PIN_MOTOR_OPEN, 1);

    uint32_t elapsed = 0;
    while (elapsed < TIMEOUT_MS) {
        sleep_ms(POLL_INTERVAL_MS);
        elapsed += POLL_INTERVAL_MS;
        if (gpio_get(PIN_SENSOR_LOCK_OPEN)) {
            sleep_ms(SETTLE_MS);
            break;
        }
    }

    gpio_put(PIN_MOTOR_OPEN, 0);
    set_lock_leds();
    send("OK");
}
