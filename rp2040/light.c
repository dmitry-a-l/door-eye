#include "light.h"
#include "hardware/gpio.h"

#define PIN_LED_WHITE         2
#define PIN_LED_RED           3
#define PIN_SENSOR_LOCK_OPEN  10

void light_leds(void) {
    bool lock_open = gpio_get(PIN_SENSOR_LOCK_OPEN);
    gpio_put(PIN_LED_WHITE, lock_open ? 0 : 1);
    gpio_put(PIN_LED_RED,   lock_open ? 1 : 0);
}
