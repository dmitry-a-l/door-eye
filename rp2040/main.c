#include "commands.h"
#include "pins.h"
#include "vpins.h"
#include "lock.h"
#include "light.h"
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#define MAX_CMD_LEN  64

int main(void) {
    stdio_usb_init();
    init_pins();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    send("READY");

    char buf[MAX_CMD_LEN];
    int  pos = 0;

    while (true) {
        poll_vpins();
        light_leds();

        int c = getchar_timeout_us(1000);
        if (c < 0) continue;

        if (c == '\n') {
            buf[pos] = 0;
            commands_handle(buf);
            pos = 0;
        } else if (c != '\r' && pos < MAX_CMD_LEN - 1) {
            buf[pos++] = (char)c;
        }
    }
}
