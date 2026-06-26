Compile project

    make clean && make compile


Upload firmware

    make install

Upload firmware from Radxa Dragon

    upload-firmware.sh

# Serial protocol

Commands are line-based over USB serial. To correlate a reply with its request
(and ignore unsolicited messages like `READY` or stale replies after a timeout),
the host may prefix a command with a correlation tag `@<id>`:

    @42 GET 8   →   @42 1

The firmware echoes the same tag on every line of the response. Commands without
a tag are answered without one. `rp-daemon` adds and verifies these tags
automatically; `rp-cmd` callers don't need to.

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


### Door action handle

PIN | Action  | Wire color
----+---------+-----------
  8 | action    | green

Action button behaviour (handled in firmware, `button.c`), decided from the lock
state at the moment of pressing:

- **Lock closed at press** → open the lock **immediately on press**, any press
  length (the open itself makes no beep). Normal 30 s autolock applies. If held
  to 2 s / 5 s it additionally arms the sticky 2 / 5 min hold and beeps once /
  twice as confirmation.
- **Lock open at press** (door closed):
  - **Short press** (released before 1 s) → close the lock (toggle).
  - **Long press** → arm the sticky **5 min** hold, beep 1 s as confirmation.

A long-press hold is **sticky**: it survives the door being opened and closed
(the automatic 10 s / 30 s autolock is suppressed), so the lock closes only after
the full 5 min. The autolock never beeps if the lock is already closed.

LED indication (firmware, `light.c`):

- Lock **closed** → white LED on, red LED off.
- Lock **open**, no autolock pending → red LED solid.
- Lock **open** with a pending autolock → the **red LED flashes the countdown**,
  more flashes per 2 s window as the deadline nears:

  | Time left | Flashes / 2 s |
  |-----------|---------------|
  | 5 min     | 1 |
  | 4 min     | 2 |
  | 3 min     | 3 |
  | 2 min     | 4 |
  | 1 min     | 5 |
  | 30 s      | 10 |
  | < 10 s    | 20 |


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

Virtual pin: `GET 100` — returns seconds remaining on the latch (0 if inactive).
Latches for **60 s** after any motion pulse; re-triggers extend the window.

## Remote relay

PIN: 28 (active-low)

Virtual pin: `GET 200` — returns `1` while active, `0` otherwise.
Latches for **1 s** on signal; **2 s** cooldown before re-triggering.
