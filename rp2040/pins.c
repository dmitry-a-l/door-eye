#include "pins.h"
#include "hardware/gpio.h"
#include "pins.h"
#include "hardware/gpio.h"

const pin_config_t PIN_CONFIG[] = {
    {     0, PIN_OUT_LOW     },  /* включить мотор закрытия замка          */
    {     1, PIN_OUT_LOW     },  /* включить мотор открытия замка           */
    {     2, PIN_OUT_LOW     },  /* включить белую подсветку замка          */
    {     3, PIN_OUT_LOW     },  /* включить красную подсветку замка        */
    {     5, PIN_OUT_LOW     },  /* зуммер vcc                              */
    {     7, PIN_IN_PULLDOWN },  /* датчик включения режима закрывания      */
    {     8, PIN_IN_PULLDOWN },  /* датчик включения режима открывания      */
    {    10, PIN_IN_PULLDOWN },  /* датчик - замок закрыт                   */
    {    11, PIN_IN_PULLDOWN },  /* датчик - замок открыт                   */
    {    13, PIN_IN_PULLDOWN },  /* датчик - дверь закрыта                  */
    { PIN26, PIN_IN_PULLDOWN },  /* датчик детектора движения               */
    { PIN28, PIN_IN_PULLUP   },  /* датчик радиореле (1-нет, 0-есть сигнал) */
};

const int PIN_CONFIG_COUNT = sizeof(PIN_CONFIG) / sizeof(PIN_CONFIG[0]);

static void init_pin(int pin, pin_mode_t mode) {
    gpio_init(pin);
    switch (mode) {
        case PIN_IN_PULLDOWN:
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_down(pin);
            break;
        case PIN_IN_PULLUP:
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_up(pin);
            break;
        case PIN_OUT_LOW:
            gpio_set_dir(pin, GPIO_OUT);
            gpio_put(pin, 0);
            break;
        case PIN_OUT_HIGH:
            gpio_set_dir(pin, GPIO_OUT);
            gpio_put(pin, 1);
            break;
    }
}

void init_pins(void) {
    for (int i = 0; i < PIN_CONFIG_COUNT; i++) {
        init_pin(PIN_CONFIG[i].pin, PIN_CONFIG[i].mode);
    }
}
