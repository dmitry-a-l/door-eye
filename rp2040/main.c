#include "commands.h"
#include "pins.h"
#include "vpins.h"
#include "lock.h"
#include "light.h"
#include "autolock.h"
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

int main(void) {
    stdio_usb_init();
    init_pins();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    send("READY");

    while (true) {
        poll_vpins();
        autolock_poll();
        light_leds();
        commands_poll();
    }
}
