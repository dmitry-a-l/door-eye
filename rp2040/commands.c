#include "commands.h"
#include "io.h"
#include "lock.h"
#include "pins.h"
#include "vpins.h"
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#define PIN_COUNT  29
#define LED_PIN    25

/* ── валидация пинов ──────────────────────────────────────────── */

extern const pin_config_t PIN_CONFIG[];
extern const int          PIN_CONFIG_COUNT;

static bool pin_valid(int pin) {
    return pin >= 0 && pin < PIN_COUNT && pin != LED_PIN;
}

static bool pin_is_output(int pin) {
    for (int i = 0; i < PIN_CONFIG_COUNT; i++) {
        if (PIN_CONFIG[i].pin == pin)
            return PIN_CONFIG[i].mode == PIN_OUT_LOW ||
                   PIN_CONFIG[i].mode == PIN_OUT_HIGH;
    }
    return false;
}

static bool pin_is_input(int pin) {
    for (int i = 0; i < PIN_CONFIG_COUNT; i++) {
        if (PIN_CONFIG[i].pin == pin)
            return PIN_CONFIG[i].mode == PIN_IN_PULLDOWN ||
                   PIN_CONFIG[i].mode == PIN_IN_PULLUP;
    }
    return false;
}

/* ── обработчики команд ───────────────────────────────────────── */

static void handle_cmd_set(int n, const char *a1, const char *a2) {
    if (n != 3)               { send("ERR - SET requires: SET <pin> <0|1>"); return; }
    int pin = atoi(a1);
    int val = atoi(a2);
    if (val != 0 && val != 1) { sendf("ERR - invalid value %d, use 0 or 1", val); return; }
    if (!pin_is_output(pin))  { sendf("ERR - pin %d is not output", pin); return; }

    gpio_put(pin, val);
    send("OK");
}

static void handle_cmd_get(int n, const char *a1) {
    if (n != 2)             { send("ERR - GET requires: GET <pin>");   return; }
    int pin = atoi(a1);
    if (pin == VPIN100)     { send(get_vpin100() ? "1" : "0");         return; }
    if (pin == VPIN200)     { send(get_vpin200() ? "1" : "0");         return; }
    if (!pin_valid(pin))    { sendf("ERR - pin %d out of range", pin); return; }
    if (!pin_is_input(pin)) { sendf("ERR - pin %d is not input", pin); return; }

    send(gpio_get(pin) ? "1" : "0");
}

static void handle_cmd_bootload(void) {
    send("OK");
    sleep_ms(10);
    reset_usb_boot(0, 0);
}

static void handle_cmd_reboot(void) {
    send("OK");
    sleep_ms(10);
    watchdog_reboot(0, 0, 0);
}

static void handle_cmd_lock(void) {
    close_lock(false);
    send("OK");
}

static void handle_cmd_unlock(void) {
    open_lock(false);
    send("OK");
}

/* ── точка входа ──────────────────────────────────────────────── */

void commands_handle(char *line) {
    size_t len = strlen(line);
    if (len && line[len - 1] == '\r') line[--len] = 0;
    if (!len) return;

    char cmd[16], a1[16], a2[16];
    a1[0] = a2[0] = 0;
    int n = sscanf(line, "%15s %15s %15s", cmd, a1, a2);
    if (n < 1) return;

    if (!strcmp(cmd, "SET"))      { handle_cmd_set(n, a1, a2); return; }
    if (!strcmp(cmd, "GET"))      { handle_cmd_get(n, a1);     return; }
    if (!strcmp(cmd, "BOOTLOAD")) { handle_cmd_bootload();     return; }
    if (!strcmp(cmd, "REBOOT"))   { handle_cmd_reboot();       return; }
    if (!strcmp(cmd, "LOCK"))     { close_lock(false);         return; }
    if (!strcmp(cmd, "UNLOCK"))   { open_lock(false);          return; }

    sendf("ERR - unknown command: %s", cmd);
}
