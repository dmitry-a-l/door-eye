#!/bin/bash

PORT=/dev/ttyACM0

# открыть порт для чтения в фоне
exec 3<$PORT 4>$PORT

rp_cmd() {
    printf '%s\n' "$1" >&4
    read -t 2 response <&3
    echo "$response"
}

rp_cmd "BOOTLOAD"
sleep 2

mount /dev/sda1 /mnt
cp rp2040_gpio_bridge.uf2 /mnt
umount /mnt

# закрыть
exec 3<&- 4>&-
