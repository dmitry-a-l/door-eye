mkdir -p /lib/firmware/edid
cp door-eye-480x480.bin /lib/firmware/edid/
vi /etc/kernel/cmdline (add drm.edid_firmware=HDMI-A-1:edid/door-eye-480x480.bin)
echo '/lib/firmware/edid/door-eye-480x480.bin' > /etc/initramfs-tools/hooks/edid

sudo update-initramfs -u
sudo kernel-install add $(uname -r) /boot/vmlinuz-$(uname -r)
