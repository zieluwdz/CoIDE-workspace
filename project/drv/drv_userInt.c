#include "drv_usrInt.h"
#include "LPC17xx.h"
#include "CoOS.h"
#include "lpc17xx_pinsel.h"

static event_cb_t intCallback;

void drv_usr_int_load	(void)
{
	PINSEL_CFG_Type PinCfg;

	PinCfg.Funcnum		= 1u;
	PinCfg.OpenDrain	= 0u;
	PinCfg.Pinmode		= PINSEL_PINMODE_PULLDOWN;
	PinCfg.Pinnum		= 10u;
	PinCfg.Portnum		= 2u;
	PINSEL_ConfigPin(&PinCfg);

	LPC_SC->EXTMODE     = (1u<<0u);	// Configure EINTx as Edge Triggered
	LPC_SC->EXTPOLAR    = (1u<<0u);	// Configure EINTx as Falling Edge
	LPC_SC->EXTINT      = (1u<<0u);	// Clear Pending interrupts
}

void drv_usr_int_unload	(void)
{

}

void drv_usr_int_open(event_cb_t pfCallback)
{
	intCallback = pfCallback;
	NVIC_SetPriority(EINT0_IRQn, ((0x01<<3)|0x01));
	LPC_SC->EXTINT = (1<<0u);	//Clear Pending interrupts in EXTINT
	NVIC_ClearPendingIRQ(EINT0_IRQn);	//Clear Pending interrupts in ICPR
	NVIC_EnableIRQ(EINT0_IRQn);
}

void drv_usr_int_close	(void)
{

}

void EINT0_IRQHandler(void)
{
	CoEnterISR();
	intCallback();
	LPC_SC->EXTINT = (1<<0u);	 //Clear Pending interrupts
	CoExitISR();
}

