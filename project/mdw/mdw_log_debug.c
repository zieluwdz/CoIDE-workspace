#include "mdw_log_debug.h"
#include "lpc17xx.h"
#include "CoOS.h"

#include "drv_lcd.h"
#include "drv_tcs.h"

#include "mdw_error.h"

#include <string.h> //strlen

#define STACK_SIZE 128

extern const char image[]; //mdw_debug_image

static const uint32_t k_xTouchMargin = 300u;
static const uint32_t k_yTouchMargin = 400u;

static const uint32_t k_xButtonLine  = 3850u;
static const uint32_t k_yButtonLine1 = 4540u;
static const uint32_t k_yButtonLine2 = 5172u;
static const uint32_t k_yButtonLine3 = 5930u;
static const uint32_t k_yButtonLine4 = 6629u;
static const uint32_t k_yButtonLine5 = 7282u;

static OS_STK	logTouchTaskStk[STACK_SIZE];
static OS_TID 	logTouchTask;
static uint32_t currentLine;

static void log_touch_task(void* pdata);

void mdw_log_load(void)
{

}

void mdw_log_open(void)
{
	logTouchTask = CoCreateTask(log_touch_task,0,0,&logTouchTaskStk[STACK_SIZE-1],STACK_SIZE);
	if(E_CREATE_FAIL == logTouchTask)
	{
		MDW_LOG_ERROR("Create Task error");
	}

	drv_lcd_load();
	drv_lcd_open();

	drv_tcs_load();
	drv_tcs_open();

	GLCD_BL_OFF();
	drv_lcd_bg_color(WHITE);
	drv_lcd_plot_picture_hor(image,0,230,80,80); // Plot LOGO
	drv_lcd_printStr_hor("Snoopy V0.1",80,225,BLACK,WHITE);
	drv_lcd_printStr_hor("Log console",80,210,BLACK,WHITE);
	drv_lcd_rectan(0u,150u,320u,0u,0u,BLACK,1u);
	GLCD_BL_ON();
	currentLine = 0u;
 }

void mdw_log_debug(char *text_nt)
{
	uint32_t currentLinePosition;

	if(strlen(text_nt)> 39u)
	{
		while(1);
	}
	if(currentLine == 8u)
	{
		currentLine = 0u;
	}
	currentLine++;
	currentLinePosition = 150u - (currentLine * 15u);
	drv_lcd_rectan(0u,currentLinePosition,320u,currentLinePosition+14,0u,BLACK,1u);
	drv_lcd_printStr_hor(text_nt,5,currentLinePosition,YELLOW,BLACK);
}

void mdw_log_close(void)
{
	//TO DO : add mdw_lcd_close and unload
	drv_tcs_close();
	drv_tcs_unload();
}

void mdw_log_unload(void)
{

}

static void log_touch_task(void* pdata)
{
	uint64_t x,y;
	while(1)
	{
		if(drv_tcs_get_status_touch())
		{
			drv_tcs_get_point(20u,&x,&y);
			if((x < (k_xButtonLine + k_xTouchMargin)) && (x > (k_xButtonLine - k_xTouchMargin)))
			{
				if((y < (k_yButtonLine1 + k_xTouchMargin)) && (y > (k_yButtonLine1 - k_xTouchMargin)))
				{
					drv_lcd_printStr_hor("Button : 1",80,195,BLACK,WHITE);
				}
				else if((y < (k_yButtonLine2 + k_xTouchMargin)) && (y > (k_yButtonLine2 - k_xTouchMargin)))
				{
					drv_lcd_printStr_hor("Button : 2",80,195,BLACK,WHITE);
				}
				else if((y < (k_yButtonLine3 + k_xTouchMargin)) && (y > (k_yButtonLine3 - k_xTouchMargin)))
				{
					drv_lcd_printStr_hor("Button : 3",80,195,BLACK,WHITE);
				}
				else if((y < (k_yButtonLine4 + k_xTouchMargin)) && (y > (k_yButtonLine4 - k_xTouchMargin)))
				{
					drv_lcd_printStr_hor("Button : 4",80,195,BLACK,WHITE);
				}
				else if((y < (k_yButtonLine5 + k_xTouchMargin)) && (y > (k_yButtonLine5 - k_xTouchMargin)))
				{
					drv_lcd_printStr_hor("Button : 5",80,195,BLACK,WHITE);
				}
				else
				{
					//Do nothing
				}

			}
			//Managed answer !!!
		}
	}
}

