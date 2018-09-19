#ifndef OLED_MANIP_DSCAO__
#define OLED_MANIP_DSCAO__
#include <stdbool.h>
#include "tm4c_gpio.h"
#include "tm4c_ssi.h"
#include "tm4c_setup.h"

struct oled_ctrl {
	enum GPIOPORT cp;
	uint8_t cmdpin, rstpin;
	char pbuf[1024];
	char abuf[1024];
	char tbuf[1024];
};

void oled_reset(struct oled_ctrl *oled);

static inline void oled_display_onoff(struct oled_ctrl *od, int on)
{
	if (on)
		od->pbuf[0] = 0xaf;
	else
		od->pbuf[0] = 0xae;
	tm4c_ssi_rwstart(0, od->pbuf, od->tbuf, 1);
}

#endif /* OLED_MANIP_DSCAO__ */
