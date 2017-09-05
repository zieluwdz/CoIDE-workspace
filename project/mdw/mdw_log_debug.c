#include "mdw_log_debug.h"
#include "lpc17xx.h"
#include "drv_lcd.h"
#include "drv_tcs.h"

#include <string.h> //strlen

extern const char image[]; //mdw_debug_image

static uint32_t currentLine;

void mdw_log_load(void)
{
	/*
	uint64_t x,y;
		while(1)
		{
			if(drv_tcs_get_status_touch())
		  	{
				drv_tcs_get_point(20u,&x,&y);
		  	}
		}
	*/
}

void mdw_log_open(void)
{

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
