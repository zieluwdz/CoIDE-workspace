#include "drv_pot.h"
#include "LPC17xx.h"
#include "CoOS.h"

#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"

uint32_t 	adcValue = 0u;
OS_FlagID	adcDoneFlag;

void ADC_IRQHandler(void);

void drv_pot_load(void)
{
	PINSEL_CFG_Type PinCfg;

	PinCfg.Funcnum		= 3u;
	PinCfg.OpenDrain	= 0u;
	PinCfg.Pinmode		= 2u;
	PinCfg.Pinnum		= 31u;
	PinCfg.Portnum		= 1u;
	PINSEL_ConfigPin(&PinCfg);
}

void drv_pot_unload(void)
{

}

void drv_pot_open(void)
{
	ADC_Init(LPC_ADC, 200000);
	ADC_IntConfig(LPC_ADC,ADC_ADINTEN5,ENABLE);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_5,ENABLE);

	NVIC_SetPriority(ADC_IRQn, ((0x01<<3)|0x01));

	adcDoneFlag = CoCreateFlag(TRUE,FALSE);
	if(E_CREATE_FAIL == adcDoneFlag)
	{
		while(1); //LOG ERROR
	}
}

void drv_pot_close(void)
{
	ADC_DeInit(LPC_ADC);
	ADC_PowerdownCmd(LPC_ADC,0u);
}

uint32_t drv_pot_getValue(void)
{
	StatusType	status;

	ADC_StartCmd(LPC_ADC,ADC_START_NOW);
	NVIC_EnableIRQ(ADC_IRQn);

	status = CoWaitForSingleFlag(adcDoneFlag,0);
	if(E_OK != status)
	{
		while(1); //LOG_ERROR
	}

	return adcValue;
}

void ADC_IRQHandler(void)
{
	CoEnterISR();

	if(ADC_ChannelGetStatus(LPC_ADC,ADC_CHANNEL_5,ADC_DATA_DONE))
	{
		adcValue = ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_5);
		NVIC_DisableIRQ(ADC_IRQn);
		isr_SetFlag(adcDoneFlag);
	}

	CoExitISR();
}
