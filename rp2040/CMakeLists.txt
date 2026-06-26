cmake_minimum_required(VERSION 3.13)

# Путь к SDK — можно переопределить через переменную среды PICO_SDK_PATH
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(rp2040_gpio_bridge C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(rp2040_gpio_bridge
    main.c
    commands.c
    vpins.c
    pins.c
    lock.c
    light.c
    io.c
    autolock.c
    button.c
)

target_link_libraries(rp2040_gpio_bridge
    pico_stdlib
    pico_stdio_usb
    pico_bootrom
    hardware_adc
    hardware_gpio
    hardware_pwm
    hardware_watchdog
)

# Включаем USB stdio, отключаем UART stdio
pico_enable_stdio_usb(rp2040_gpio_bridge 1)
pico_enable_stdio_uart(rp2040_gpio_bridge 0)

# Генерируем .uf2 и .bin
pico_add_extra_outputs(rp2040_gpio_bridge)
