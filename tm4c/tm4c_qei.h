#ifndef TM4C_QEI_DSCAO__
#define TM4C_QEI_DSCAO__
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_qei.h"
#include "driverlib/sysctl.h"
#include "driverlib/qei.h"
#include "driverlib/rom.h"

struct qei_port {
	uint32_t base;
	uint16_t err;
	int16_t maxpos;
	int16_t minpos;
};

void tm4c_qei_setup(int port, uint32_t pos, int max, int min);

void qei0_isr(void);
void qei1_isr(void);

int tm4c_qei_getpos(int port);
void tm4c_qei_setpos(int port, int pos);
void tm4c_qei_velconf(int port, uint32_t tintv);
void tm4c_qei_velstart(int port);
void tm4c_qei_velstop(int port);
uint32_t tm4c_qei_velget(int port);
uint32_t tm4c_qei_getctl(int port);
int tm4c_qei_velproc(int port);

#endif /* TM4C_QEI_DSCAO__ */
