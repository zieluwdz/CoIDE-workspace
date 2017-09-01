#include "drv_buzz.h"
#include "LPC17xx.h"

#include "lpc17xx_clkpwr.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_nvic.h"

static uint32_t TmrCounter;

void TIMER0_IRQHandler (void);

void drv_buzz_load(void)
{
	LPC_GPIO3->FIOMASK	= 0x00;
	LPC_PINCON->PINSEL7	&= ~(0xffff); 	//GPIO
	LPC_GPIO3->FIOSET	= (1u << 26u);  //Set init value
	LPC_GPIO3->FIODIR 	|= (1UL<<26u);	//Output
}

void drv_buzz_open(void)
{

}

void drv_buzz_on(void)
{
	LPC_GPIO3->FIOCLR	= (1u << 26u);
}

void drv_buzz_off(void)
{
	LPC_GPIO3->FIOSET	= (1u << 26u);
}

void drv_buzz_generateStart(uint32_t delay_us)
{
	TIM_TIMERCFG_Type TMR0_Cfg;
	TIM_MATCHCFG_Type TMR0_Match;
	TmrCounter = delay_us / 2u;
	/* On reset, Timer0/1 are enabled (PCTIM0/1 = 1), and Timer2/3 are disabled (PCTIM2/3 = 0).*/
	/* Initialize timer 0, prescale count time of 100uS */
	TMR0_Cfg.PrescaleOption = TIM_PRESCALE_USVAL;
	TMR0_Cfg.PrescaleValue = 1u;
	/* Use channel 0, MR0 */
	TMR0_Match.MatchChannel = 0;
	/* Enable interrupt when MR0 matches the value in TC register */
	TMR0_Match.IntOnMatch = ENABLE;
	/* Enable reset on MR0: TIMER will reset if MR0 matches it */
	TMR0_Match.ResetOnMatch = TRUE;
	/* Don't stop on MR0 if MR0 matches it*/
	TMR0_Match.StopOnMatch = FALSE;
	/* Do nothing for external output pin if match (see cmsis help, there are another options) */
	TMR0_Match.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	/* Set Match value, count value of 5u (5 * 1uS = 5us --> 500 kHz) */
	TMR0_Match.MatchValue = 1u;
	/* Set configuration for Tim_config and Tim_MatchConfig */
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TMR0_Cfg);
	TIM_ConfigMatch(LPC_TIM0, &TMR0_Match);

	/* Preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(TIMER0_IRQn, ((0x01<<3)|0x01));
	/* Enable interrupt for timer 0 */
	NVIC_EnableIRQ(TIMER0_IRQn);
	/* Start timer 0 */
	TIM_Cmd(LPC_TIM0, ENABLE);
	LPC_GPIO3->FIOCLR	= (1u << 26u);
}

void drv_buzz_generateStop()
{
	LPC_GPIO3->FIOSET	= (1u << 26u);
	NVIC_DisableIRQ(TIMER0_IRQn);
	TIM_Cmd(LPC_TIM0, DISABLE);
}

void drv_buzz_close(void)
{

}

void drv_buzz_unload(void)
{
	LPC_GPIO3->FIOSET	= (1u << 26u);
	LPC_GPIO2->FIODIR 	&=~ (1UL<<26u); //Input
}

void TIMER0_IRQHandler (void)
{
	static uint32_t irqNum = 0u;
	TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT);
	if(TmrCounter < irqNum)
	{
		LPC_GPIO3->FIOPIN	^= (1<<26u);
		irqNum = 0u;
	}
	else
	{
		irqNum++;
	}

}
