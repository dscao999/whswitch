#ifndef TM4C_PWM_DSCAO__
#define TM4C_PWM_DSCAO__
#include <stdint.h>
#include <inc/hw_pwm.h>

struct tm4c_pwm {
	uint32_t base;
	uint32_t intrs[4];
};

struct tm4c_pwm * tm4c_pwm_init(uint32_t mpwm);

int tm4c_pwm_set(struct tm4c_pwm *pwm, int gen, int freq);

void tm4c_pwm_setfreq(struct tm4c_pwm *pwm, int gen, int freq);

static inline void tm4c_pwm_start(struct tm4c_pwm *pwm, int gen)
{
	HWREG(pwm->base+PWM_GEN_0_OFFSET+PWM_O_X_CTL) |= 1;
	HWREG(pwm->base+PWM_O_ENABLE) |= PWM_ENABLE_PWM0EN;
}

static inline void tm4c_pwm_stop(struct tm4c_pwm *pwm, int gen)
{
	HWREG(pwm->base+PWM_O_ENABLE) &= ~PWM_ENABLE_PWM0EN;
	HWREG(pwm->base+PWM_GEN_0_OFFSET+PWM_O_X_CTL) &= ~1;
}

static inline uint32_t tm4c_pwm_get_intrs(struct tm4c_pwm *pwm, int gen)
{
	return pwm->intrs[gen];
}

void pwm_0_gen_0_intr(void);  /* PWM0 GEn 0 Int handler */

#endif /* TM4C_PWM_DSCAO__ */
