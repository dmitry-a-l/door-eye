mkdir -p /lib/firmware/edid
cp door-eye-480x480.bin /lib/firmware/edid/

В /etc/kernel/cmdline оставьте оба:
... drm.edid_firmware=HDMI-A-1:edid/door-eye-480x480.bin video=HDMI-A-1:480x480@60e ...

echo 'edid/door-eye-480x480.bin' | sudo tee -a /etc/initramfs-tools/hooks/edid-files
# либо проще — через FIRMWARE_DIRS:
echo 'FRAMEBUFFER=y' | sudo tee -a /etc/initramfs-tools/conf.d/edid

sudo update-initramfs -u
sudo kernel-install add $(uname -r) /boot/vmlinuz-$(uname -r)
