#include "io.h"
#include "lock.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define PIN_MOTOR_OPEN         0
#define PIN_MOTOR_CLOSE        1

#define PIN_SENSOR_LOCK_OPEN   10   /* 1 - замок открыт,  0 - замок закрыт */
#define PIN_SENSOR_DOOR_CLOSE  13   /* 1 - дверь закрыта, 0 - дверь открыта */

#define POLL_INTERVAL_MS       100
#define SETTLE_MS              300
#define TIMEOUT_MS             3000

void close_lock(bool silent) {
    if (!gpio_get(PIN_SENSOR_DOOR_CLOSE)) {
        if (!silent) send("ERR - door is open");
        return;
    }
    if (!gpio_get(PIN_SENSOR_LOCK_OPEN)) {
        if (!silent) send("ERR - lock already closed");
        return;
    }

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
    gpio_put(PIN_MOTOR_OPEN, 1);
    sleep_ms(100);
    gpio_put(PIN_MOTOR_OPEN, 0);

    if (!silent) send("OK");
}

void open_lock(bool silent) {
    if (!gpio_get(PIN_SENSOR_DOOR_CLOSE)) {
        if (!silent) send("ERR - door is open");
        return;
    }
    if (gpio_get(PIN_SENSOR_LOCK_OPEN)) {
        if (!silent) send("ERR - lock already open");
        return;
    }

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
    gpio_put(PIN_MOTOR_CLOSE, 1);
    sleep_ms(100);
    gpio_put(PIN_MOTOR_CLOSE, 0);

    if (!silent) send("OK");
}
