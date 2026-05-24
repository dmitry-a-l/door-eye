Compile project

    make clean && make compile


Upload firmware

    make install

Upload firmware from Radxa Dragon

    upload-firmware.sh

# PINs description

## Actions

### Lock

PIN | Action                | Wire color
----+-----------------------+--------------
  0 | rotate lock to open   | blue
  1 | rotate lock to close  | green
  2 | light a white LED     | white-orange
  3 | light a red LED       | orange

### Buzzer

PIN | Action
----+-----------------
  5 | beep the buzzer


### Door lock handle

PIN | Action  | Wire color
----+---------+-----------
  7 | unlock  | blue
  8 | lock    | green


## Sensors

### Lock

PIN | Sensor       | Wire color
----+--------------+-----------
 10 | lock opened  | white-blue
 11 | lock closed  | white-green


### Door

PIN | Sensor       | Wire color
----+--------------+-----------
 13 | door closed  | white-brown


## Radar detector

PIN: 26

## Remote relay

PIN: 28
