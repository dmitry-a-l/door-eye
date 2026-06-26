#pragma once

#include <stdint.h>

void autolock_poll(void);

/* Schedule the lock to close after delay_ms from now, overriding any pending
 * autolock. */
void autolock_set(uint32_t delay_ms);

/* Milliseconds until the scheduled auto-close, or 0 if none is pending. */
uint32_t autolock_remaining_ms(void);
