#include "drv_led.h"
#include "LPC17xx.h"
#include "CoOS.h"

#include <stdlib.h>
#include <math.h>

#include "mdw_error.h"

#define STACK_SIZE 128

typedef struct
{
	drv_led_mode	mode;
	uint32_t		delay_ms;
	uint8_t			ledMask;
}ledConfig;

static OS_MutexID	ledConfigMutex; //Protection led Config
static OS_STK		ledTaskStk[STACK_SIZE];
static OS_TID 		ledTask;
static ledConfig	runningConf;

static void led_task 	(void* pdata);

void drv_led_load(void)
{
	LPC_GPIO2->FIOMASK	= 0x00;
	LPC_PINCON->PINSEL4 &= ~(0xffff); //GPIO
	LPC_GPIO2->FIODIR 	|= 0xff; //Output
	LPC_GPIO2->FIOCLR	 = 0xff; //Reset the init value
}

void drv_led_open(void)
{
	ledTask = 0u;

	ledConfigMutex = CoCreateMutex();
	if(E_CREATE_FAIL == ledConfigMutex)
	{
		MDW_LOG_ERROR("Create Mutex error");
	}
}

void drv_led_start(drv_led_mode mode, uint32_t delay_ms)
{
	CoEnterMutexSection(ledConfigMutex);
	runningConf.mode		= mode;
	runningConf.delay_ms	= delay_ms;
	CoLeaveMutexSection(ledConfigMutex);

	if(0u != ledTask)
	{
		while(1); 	//LOG_ERROR
	}

	ledTask = CoCreateTask(led_task,&runningConf,0,&ledTaskStk[STACK_SIZE-1],STACK_SIZE);
	if(E_CREATE_FAIL == ledTask)
	{
		while(1); 	//LOG_ERROR
	}
}

void drv_led_stop(void)
{
	StatusType status = CoDelTask(ledTask);
	if(E_OK != status)
	{
		MDW_LOG_ERROR("Delete Task error");
	}

	ledTask = 0u;
}

void drv_led_speed(uint32_t delay_ms)
{
	CoEnterMutexSection(ledConfigMutex);
	runningConf.delay_ms = delay_ms;
	CoLeaveMutexSection(ledConfigMutex);
}

void drv_led_set(uint8_t valueLed)
{
	LPC_GPIO2->FIOSET  = valueLed;
}

void drv_led_clear(uint8_t valueLed)
{
	LPC_GPIO2->FIOCLR  = valueLed;
}

void drv_led_setOne(uint8_t singleLed)
{
	LPC_GPIO2->FIOSET = (1u << singleLed);
}

void drv_led_clearOne(uint8_t singleLed)
{
	LPC_GPIO2->FIOCLR = (1u << singleLed);
}

void drv_led_close(void)
{

}

void drv_led_unload(void)
{
	LPC_GPIO2->FIODIR 	&= ~(0xff); //Input
}

static void led_task(void* pdata)
{
	ledConfig *pRunningConf = pdata;
	static uint32_t iter = 0x01u;

	for (;;)
	{
		drv_led_clear(0xff);
		switch(pRunningConf->mode)
		{
			case k_OneWave:
				drv_led_set(iter);
				iter = iter << 1u;
				if(iter > 128)
				{
					iter = 0x01u;
				}
				break;
			case k_Wave:
				drv_led_set(pow(2,iter) - 1u);
				iter++;
				if(iter > 8u)
				{
					iter = 0x01u;
				}
				break;
			default:
				MDW_LOG_ERROR("switch case default!");
				//Code to execute if <variable> does not equal the value following any of the cases
				break;
		}
		CoTickDelay(pRunningConf->delay_ms);
	}
}
