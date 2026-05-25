#!/bin/bash
set -euo pipefail

rp-cmd "BOOTLOAD"
sleep 2
mount /dev/sda1 /mnt
cp rp2040_gpio_bridge.uf2 /mnt
umount /mnt
reboot
