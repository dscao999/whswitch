#ifndef OLED_MANIP_DSCAO__
#define OLED_MANIP_DSCAO__
#include <stdbool.h>
#include "tm4c_gpio.h"
#include "tm4c_ssi.h"
#include "tm4c_setup.h"

#define NULL	((void *)0)

struct oled_ctrl {
	uint16_t width, height;
	enum GPIOPORT cp;
	uint8_t ssi, cmdpin, rstpin;
	char pbuf[1024] __attribute__ ((aligned (4)));
	char abuf[1024];
	char tbuf[1024];
};

void oled_fill_display(struct oled_ctrl *od, int color);
void oled_reset(struct oled_ctrl *oled, int color);
void oled_init(struct oled_ctrl *od, int ssiport, int color);

static inline void oled_display_onoff(struct oled_ctrl *od, int on)
{
	char cmd, res;

	if (on)
		cmd = 0xaf;
	else
		cmd = 0xae;
	while (tm4c_ssi_rwst(od->ssi))
		tm4c_waitint();
	tm4c_ssi_rwstart(od->ssi, &cmd, &res, 1, NULL);
	tm4c_ssi_wdma(od->ssi);
}

#endif /* OLED_MANIP_DSCAO__ */
