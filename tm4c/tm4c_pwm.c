#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_memmap.h>
#include <inc/hw_pwm.h>
#include <inc/hw_ints.h>
#include <inc/hw_types.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/pwm.h>
#include "miscutils.h"
#include "tm4c_pwm.h"

extern const uint32_t HZ;
static int pwmfreq = 0;
static int clock_set = 0;

static struct tm4c_pwm pwmm[2] = {
	{
		.base = PWM0_BASE,
	},
	{
		.base = PWM1_BASE
	}
};

#ifdef PWM_INT_FAULT_M
#undef PWM_INT_FAULT_M
#define PWM_INT_FAULT_M	0x030000
#endif
#define PWM_INT_GEN_M	0x0f

struct tm4c_pwm * tm4c_pwm_init(uint32_t mpwm)
{
	struct tm4c_pwm *pwm = pwmm + mpwm;
	uint32_t perip, intr;

	switch(mpwm) {
	case 0:
		ROM_GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
		ROM_GPIOPinConfigure(GPIO_PB6_M0PWM0);
		perip = SYSCTL_PERIPH_PWM0;
		break;
	case 1:
		perip = SYSCTL_PERIPH_PWM1;
		break;
	default:
		return 0;
	}
	ROM_SysCtlPeripheralEnable(perip);
	while (!ROM_SysCtlPeripheralReady(perip))
                        ;
	HWREG(pwm->base+PWM_O_INTEN) &= ~(PWM_INT_FAULT_M | PWM_INT_GEN_M);
	HWREG(pwm->base+PWM_GEN_0_OFFSET) = 0;
	HWREG(pwm->base+PWM_GEN_1_OFFSET) = 0;
	HWREG(pwm->base+PWM_GEN_2_OFFSET) = 0;
	HWREG(pwm->base+PWM_GEN_3_OFFSET) = 0;
	intr = HWREG(pwm->base+PWM_GEN_0_OFFSET+PWM_O_X_ISC);
	HWREG(pwm->base+PWM_GEN_0_OFFSET+PWM_O_X_ISC) = intr;

	if (mpwm == 0) {
		ROM_IntPrioritySet(INT_PWM0_0, 0x40);
		ROM_IntPrioritySet(INT_PWM0_1, 0x40);
		ROM_IntPrioritySet(INT_PWM0_2, 0x40);
		ROM_IntPrioritySet(INT_PWM0_3, 0x40);
	} else {
		ROM_IntPrioritySet(INT_PWM1_0, 0x40);
		ROM_IntPrioritySet(INT_PWM1_1, 0x40);
		ROM_IntPrioritySet(INT_PWM1_2, 0x40);
		ROM_IntPrioritySet(INT_PWM1_3, 0x40);
	}
	if (unlikely(clock_set == 0)) {
		ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_64);
		pwmfreq = HZ / 64;
		clock_set = 1;
	}

	return pwm;
}

int tm4c_pwm_set(struct tm4c_pwm *pwm, int gen, int freq)
{
	uint32_t count, genbase, act_a, int_a, gen_int;
	int intr_num;

	count = pwmfreq / freq;
	if (count > 0x0ffff)
		return 1;

	switch(gen) {
	case 0:
		genbase = PWM_GEN_0_OFFSET;
		act_a = PWM_0_GENA_ACTCMPAD_ZERO|PWM_0_GENA_ACTLOAD_ONE;
		int_a = PWM_0_INTEN_INTCNTZERO;
		gen_int = PWM_INT_GEN_0;
		if (pwm == pwmm)
			intr_num = INT_PWM0_0;
		else
			intr_num = INT_PWM1_0;
		break;
	case 1:
		genbase = PWM_GEN_1_OFFSET;
		act_a = PWM_1_GENA_ACTCMPAD_ZERO|PWM_1_GENA_ACTLOAD_ONE;
		int_a = PWM_1_INTEN_INTCNTZERO;
		gen_int = PWM_INT_GEN_1;
		if (pwm == pwmm)
			intr_num = INT_PWM0_1;
		else
			intr_num = INT_PWM1_1;
		break;
	case 2:
		genbase = PWM_GEN_2_OFFSET;
		act_a = PWM_2_GENA_ACTCMPAD_ZERO|PWM_2_GENA_ACTLOAD_ONE;
		int_a = PWM_2_INTEN_INTCNTZERO;
		gen_int = PWM_INT_GEN_2;
		if (pwm == pwmm)
			intr_num = INT_PWM0_2;
		else
			intr_num = INT_PWM1_2;
		break;
	case 3:
		genbase = PWM_GEN_3_OFFSET;
		act_a = PWM_3_GENA_ACTCMPAD_ZERO|PWM_3_GENA_ACTLOAD_ONE;
		int_a = PWM_3_INTEN_INTCNTZERO;
		gen_int = PWM_INT_GEN_3;
		if (pwm == pwmm)
			intr_num = INT_PWM0_3;
		else
			intr_num = INT_PWM1_3;
		break;
	default:
		return 2;
	}
	pwm->intrs[gen] = 0;
	HWREG(pwm->base+genbase+PWM_O_X_LOAD) = count;
	HWREG(pwm->base+genbase+PWM_O_X_CMPA) = (count >> 1);
	HWREG(pwm->base+genbase+PWM_O_X_GENB) = 0;
	HWREG(pwm->base+genbase+PWM_O_X_GENA) = act_a;
	HWREG(pwm->base+genbase+PWM_O_X_INTEN) = int_a;
	HWREG(pwm->base+PWM_O_INTEN) |= gen_int;
	ROM_IntEnable(intr_num);

	return 0;
}

static void pwm_intr(struct tm4c_pwm *pwm, int gbase)
{
	uint32_t intr;

	intr = HWREG(pwm->base+gbase+PWM_O_X_ISC);
	HWREG(pwm->base+gbase+PWM_O_X_ISC) = intr;
	intr = HWREG(pwm->base+gbase+PWM_O_X_ISC);
}

void pwm_0_gen_0_intr(void)
{
	struct tm4c_pwm *pwm = pwmm;

	pwm_intr(pwm, PWM_GEN_0_OFFSET);
	pwm->intrs[0] += 1;
}

void tm4c_pwm_setfreq(struct tm4c_pwm *pwm, int gen, int freq)
{
	int gbase, count, sync;

	switch(gen) {
	case 0:
		gbase = PWM_GEN_0_OFFSET;
		sync = 1;
		break;
	case 1:
		gbase = PWM_GEN_1_OFFSET;
		sync = 2;
		break;
	case 2:
		gbase = PWM_GEN_2_OFFSET;
		sync = 4;
		break;
	case 3:
		gbase = PWM_GEN_3_OFFSET;
		sync = 8;
		break;
	default:
		return;
	}
	count = pwmfreq / freq;
	HWREG(pwm->base+gbase+PWM_O_X_LOAD) = count;
	HWREG(pwm->base+gbase+PWM_O_X_CMPA) = (count >> 1);
	if ((HWREG(pwm->base+gbase+PWM_O_X_CTL) & 1) == 0)
		HWREG(pwm->base+PWM_O_SYNC) = sync;
}
