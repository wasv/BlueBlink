/* Blinks an LED with PWM on Timer 3 on an stm32f103 'Blue Pill' dev board.
 * Uses PC13 for status LED, PB0 as PWM LED
 *  - William A Stevens V (wasv)
 */
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define CLOCK_FREQ  8000000 // 8MHz timer clock
//#define SAMPL_FREQ    32000 // 32KHz audio
#define SAMPL_FREQ       1 // 1Hz visual
#define SAMPL_RESN     256 // 8 bit resolution
#define SAMPL_PSCL (CLOCK_FREQ/(SAMPL_RESN*SAMPL_FREQ))

static void gpio_setup(void)
{
	/* Enable GPIOC clock. */
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Set GPIO13 (in GPIOC) to 'output push-pull'. */
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}

static void tim3_setup(void)
{
	/* Enable Timer 3. */
	rcc_periph_clock_enable(RCC_TIM3);

	/* Configure Timer 3 w/ No clock divider, edge triggered, upcounting */
	timer_set_mode(TIM3,TIM_CR1_CKD_CK_INT,TIM_CR1_CMS_EDGE,TIM_CR1_DIR_UP);
	/* Prescale CLOCK_FREQ to (SAMPL_FREQ*SAMPL_RESN) */
	timer_set_prescaler(TIM3,SAMPL_PSCL);

	/* Configure Timer 3 Output 3 Active when counter < compare value */
	timer_set_oc_mode(TIM3, TIM_OC3, TIM_OCM_PWM1);

	/* Configure Timer 3 Output 3 to be active high */
	timer_set_oc_polarity_high(TIM3, TIM_OC3);

	/* Set 256 tick period. */
	timer_enable_preload(TIM3);
	timer_set_period(TIM3, SAMPL_RESN);
	timer_enable_update_event(TIM3);

	/* Start at 10% duty cycle. */
	timer_set_oc_value(TIM3, TIM_OC3, SAMPL_RESN/10);

	/* Enable Timer */
	timer_enable_counter(TIM3);

	/* Enable GPIOB clock. */
	rcc_periph_clock_enable(RCC_GPIOB);
	/* Configure Output Pin B0 as TIM3C3 */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO0);

	/* Enable Output compare mode */
	timer_enable_oc_output(TIM3, TIM_OC3);
}

int main(void)
{
	tim3_setup();
	gpio_setup();

	/* Blink the LED (PC13) on the board. */
	while (1) {
		gpio_toggle(GPIOC, GPIO13);	/* LED on/off */
		for (int i = 0; i < 800000; i++)	/* Wait a bit. */
			__asm__("nop");
	}

	return 0;
}
