/* Blinks an LED with PWM on Timer 3 on an stm32f103 'Blue Pill' dev board.
 * Uses PC13 for status LED, PB0 as PWM LED
 *  - William A Stevens V (wasv)
 */
#include "samples.h"
#include <stdint.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

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
	timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	/* Prescale CLOCK_FREQ to (SAMPL_FREQ*SAMPL_RESN) */
	timer_set_prescaler(TIM3, SAMPL_PSCL);

	/* Configure Timer 3 Output 3 Active when counter < compare value */
	timer_set_oc_mode(TIM3, TIM_OC3, TIM_OCM_PWM1);

	/* Configure Timer 3 Output 3 to be active high */
	timer_set_oc_polarity_high(TIM3, TIM_OC3);

	/* Set 256 tick period. */
	//timer_enable_preload(TIM3);
	timer_set_period(TIM3, SAMPL_RESN);

	/* Start at 10% duty cycle. */
	timer_set_oc_value(TIM3, TIM_OC3, 0.5*SAMPL_RESN);

	/* Enable DMA request on update */
	timer_update_on_overflow(TIM3);
	timer_set_dma_on_update_event(TIM3);
	timer_enable_irq(TIM3, TIM_DIER_TDE | TIM_DIER_UDE);
	timer_enable_update_event(TIM3);

	/* Clear fields of DMA control register */
	TIM_DCR(TIM3) &= ~(TIM_BDTR_DBL_MASK | TIM_BDTR_DBA_MASK);
	/* DMA Burst Length is 1, leave field cleared. */
	// TIM_DCR(TIM3) |= (TIM_BDTR_DBL_MASK & 0);
	/* DMA writes should go to CCR3 (byte offset 0x3C, register 15) */
	TIM_DCR(TIM3) |= (TIM_BDTR_DBA_MASK & 0x0F);
	
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

static void dma_setup(void) {
	/* Enable DMA 1. */
	rcc_periph_clock_enable(RCC_DMA1);

	/* Reset DMA 1 Channel 5 */
	dma_channel_reset(DMA1, DMA_CHANNEL3);

	/* Reading from memory to perihperal */
	dma_set_read_from_memory(DMA1, DMA_CHANNEL3);

	/* Writing to same 16 bit address each time */
	dma_disable_peripheral_increment_mode(DMA1, DMA_CHANNEL3);
	dma_set_peripheral_size(DMA1, DMA_CHANNEL3, DMA_CCR_PSIZE_16BIT);
	/* Specify peripheral register */
	dma_set_peripheral_address(DMA1, DMA_CHANNEL3, (uint32_t)(&TIM_DMAR(TIM3)));

	/* Reading from next 8 bit address each time */
	dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL3);
	dma_set_memory_size(DMA1, DMA_CHANNEL3, DMA_CCR_MSIZE_8BIT);
	/* Specify buffer location and length */
	dma_set_memory_address(DMA1, DMA_CHANNEL3, (uint32_t)(&samples));
	dma_set_number_of_data(DMA1, DMA_CHANNEL3, NUM_SAMPLES);

	/* Enable circular mode, loops to start when done. */
	dma_enable_circular_mode(DMA1, DMA_CHANNEL3);

	/* Set DMA Priority to high */
	dma_set_priority(DMA1, DMA_CHANNEL3, DMA_CCR_PL_HIGH);

	dma_enable_channel(DMA1, DMA_CHANNEL3);
}

int main(void)
{
	dma_setup();
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
