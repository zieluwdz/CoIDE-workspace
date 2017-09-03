#include "main.h"

#include "CoOS.h"
#include "LPC17xx.h"

#include "drv_led.h"
#include "drv_buzz.h"
#include "drv_pot.h"
#include "drv_usrInt.h"
#include "drv_lcd.h"
#include "drv_tcs.h"

#include "mdw_error.h"
#include "mdw_log_debug.h"

#define STACK_SIZE 128

static OS_STK	mainTaskStk[STACK_SIZE];
static OS_TID 	mainTask;

static void main_task(void* pdata);
static void button_interrupt(void);

int main(void)
{
	drv_led_load();
	drv_buzz_load();
	drv_pot_load();
	drv_usr_int_load();

	mdw_log_load();

	CoInitOS (); //Init OS

	mainTask = CoCreateTask(main_task,0,0,&mainTaskStk[STACK_SIZE-1],STACK_SIZE);
	if(E_CREATE_FAIL == mainTask)
	{
		MDW_LOG_ERROR("Create Task error");
	}

	CoStartOS ();	//Start OS
	while(1);   	//CoOS is running
    return 0;
}

static void main_task(void* pdata)
{
	long x1=152,y1=119,x2=159,y2=126;
	char num1,num2,num3;

	drv_led_open();
	drv_buzz_open();
	drv_pot_open();
	drv_usr_int_open(button_interrupt);

	mdw_log_open();

	mdw_log_debug("It is working well ;) well done ");

	//Start up procedure
	drv_led_start(k_OneWave,50u);
	CoTimeDelay(0u,0u,0u,400u);
	drv_led_stop();

	drv_led_start(k_Wave,500u);

	//Application is in main while
	for (;;)
	{
		uint32_t speed = drv_pot_getValue();
		if(speed < 50u) speed = 50u;
		drv_led_speed(speed);

		if(Get_Status_Touch())         	    													// return (1) if tap touch screen is detected.
			{
			  TCS_Get_Point(20); 				// Calculate Position Address Display Keep result in valiable X,Y
			  drv_led_stop();
		    }
	}

	//TO DO
	//add close
	//add unload
}

static void button_interrupt(void)
{
	LPC_GPIO3->FIOPIN ^= (1<<26u);
}
