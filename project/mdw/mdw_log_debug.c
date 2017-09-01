#include "mdw_log_debug.h"
#include "lpc17xx.h"
#include "drv_lcd.h"

#include <string.h> //strlen

extern const char image[]; //mdw_debug_image

static uint32_t currentLine;

void mdw_log_load(void)
{

}

void mdw_log_open(void)
{							// Start Initial GLCD
  Initial_Hardware();			// Init SPI for communication between LCD and LPC
  Initial_GLCD_Hor();			// Init LCD

  GLCD_BL_OFF();
  bg_color(WHITE);
  plot_picture_hor(image,0,230,80,80); // Plot LOGO
  lcd_printStr_hor("Doggy V0.0",80,225,BLACK,WHITE);
  lcd_printStr_hor("Log console",80,210,BLACK,WHITE);
  Rectan(0u,150u,320u,0u,0u,BLACK,1u);
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
	Rectan(0u,currentLinePosition,320u,currentLinePosition+14,0u,BLACK,1u);
	lcd_printStr_hor(text_nt,5,currentLinePosition,YELLOW,BLACK);
}

void mdw_log_close(void)
{

}

void mdw_log_unload(void)
{

}
