#pragma once

#define PIN26  26
#define PIN28  28

typedef enum {
    PIN_IN_PULLDOWN,
    PIN_IN_PULLUP,
    PIN_OUT_LOW,
    PIN_OUT_HIGH,
} pin_mode_t;

typedef struct {
    int        pin;
    pin_mode_t mode;
} pin_config_t;

extern const pin_config_t PIN_CONFIG[];
extern const int          PIN_CONFIG_COUNT;

void init_pins(void);
