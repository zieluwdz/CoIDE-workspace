#include "drv_lcd.h"

#include "lpc17xx.h"
#include <CoOS.h>

#include "lcd/type.h"
#include "math.h"

/************/
/* Delay ms */
/************/
void delay_ms(unsigned long ms)
 {
  CoTimeDelay(0,0,0,ms);
 }


/******************************************/
/* Config Hardware Interface GLCD & Touch */
/* (LCD Driver SPFD5408A + Touch ADS7846) */
/* GLCD(ET-TFT240320P) SPI Mode Interface */
/* -> P0.6(GPIO:Out)    = CS GLCD         */
/* -> P0.7(SCK1:SSP1)   = SCL	GLCD	  */
/* -> P0.8(MISO1:SSP1)  = SDO GLCD		  */
/* -> P0.9(MOSI1:SSP1)  = SDI	GLCD	  */
/* -> P4.28(GPIO:Out)   = BL GLCD		  */
/******************************************/
void Initial_Hardware()
{
  /* Config P0.6,P0.7,P0.8,P0.9 to SSP1 For Control GLCD */
  // P0.6(GPIO:Out)   = CS GLCD
  // P0.7(SCK1:SSP1)  = SCL	GLCD
  // P0.8(MISO1:SSP1) = SDO GLCD
  // P0.9(MOSI1:SSP1) = SDI	GLCD
  // P4.28(GPIO:Out)  = BL GLCD
  LPC_PINCON->PINSEL9 &= ~(3 << 24); 														// Reset P4.28 Mode = GPIO
  LPC_GPIO4->FIODIR   |=  (1UL<<28);       													// Pin P4.28 = Output(GLCD Backlight)
  LPC_GPIO4->FIOPIN   &= ~(1UL<<28);       													// Turn-OFF GLCD Backlight

  LPC_PINCON->PINSEL0 &= ~(3UL<<12); 														// Reset P0.6 Mode = GPIO
  LPC_GPIO0->FIODIR   |=  (1 <<  6);    													// Pin P0.6 is GPIO output(CS GLCD)
  LPC_GPIO0->FIOSET    =  (1 <<  6);    													// Set P0.6 = High

  LPC_PINCON->PINSEL0 &= ~(3UL<<14); 														// Reset P0.7 Mode = GPIO
  LPC_PINCON->PINSEL0 |=  (2UL<<14); 														// Select P0.7 = SCK1(SSP1)
  LPC_PINCON->PINSEL0 &= ~(3UL<<16); 														// Reset P0.8 Mode = GPIO
  LPC_PINCON->PINSEL0 |=  (2UL<<16); 														// Select P0.8 = MISO1(SSP1)
  LPC_PINCON->PINSEL0 &= ~(3UL<<18); 														// Reset P0.9 Mode = GPIO
  LPC_PINCON->PINSEL0 |=  (2UL<<18); 														// Select P0.9 = MOSI1(SSP1)

  LPC_SC->PCONP       |=  (1 << 10);    													// Enable power to SSP1 block
  LPC_SC->PCLKSEL0 	  |=  (2 << 20); 														// SSP1 clock = CCLK/2 (36MHz)
  LPC_SSP1->CPSR 	  = 2; 																		// Clock Rate = 18MH

  LPC_SSP1->CR0        =  (   1 << 7) | 													// CPHA = 1
                          (   1 << 6) | 													// CPOL = 1
                          (   0 << 4) | 													// Frame format = SPI
                          ((8-1)<< 0) ; 													// Data size = 8 bits
  LPC_SSP1->CR1        =  (   1 << 1);  													// Enable SSP1
}

/****************************/
/* GLCD SPI Sent Data 8 bit */
/****************************/
unsigned char GLCD_SPI_Read_Write(unsigned char DataByte)
{
  LPC_SSP1->DR = DataByte;
  while (LPC_SSP1->SR & (1 << 4));															// Wait for transfer to finish
  return (LPC_SSP1->DR);                													// Return received value
}

/******************************************************/
/* Write Address Command(Index Reg.)(Use Device ID=0) */
/******************************************************/
void GLCD_Write_Command(unsigned char GLCD_Command)
{
  GLCD_CS_LOW();                     														// Enable GLCD Interface

  GLCD_SPI_Read_Write(0x70);																// Sent Byte 1 = [Device ID Code:01110[0]]+[RS:0] + [R/W:0]
  GLCD_SPI_Read_Write(0x00);																// Sent Byte 2 = data 8 bit High Index Reg.: 0x00
  GLCD_SPI_Read_Write(GLCD_Command);														// Sent Byte 3 = data 8 bit Low index reg. : cmm

  GLCD_CS_HIGH();                           												// Disable GLCD Interface
}

/***************************************/
/* Write data to LCD (Use Device ID=0) */
/***************************************/
void GLCD_Write_Data(unsigned int GLCD_Data)
{
  GLCD_CS_LOW();                     														// Enable GLCD Interface

  GLCD_SPI_Read_Write(0x72);                												// Byte 1 = [Device ID Code:01110[0]]+[RS:1] + [R/W:0]
  GLCD_SPI_Read_Write(GLCD_Data >> 8);      												// Byte 2 = Data 8 bit High
  GLCD_SPI_Read_Write(GLCD_Data);           												// Byte 3 = Data 8 bit Low

  GLCD_CS_HIGH();                           												// Disable GLCD Interface
}

/********************************/
/* Initial GLCD : ET-TFT240320P */
/*      (Driver SPFD5408A)      */
/********************************/
void Initial_GLCD_Hor(void)
{
  unsigned long cnt;

  GLCD_Write_Command(0x00);	 																// Command Start Oscillater
  GLCD_Write_Data(0x0001);	 																// Enable the oscillator.
  delay_ms(10);    																			// Delay for 10ms while the oscillator stabilizes.

  // Display Setting
  GLCD_Write_Command(0x01);	 																// Configure the output drivers.
  GLCD_Write_Data(0x0100);
  GLCD_Write_Command(0x02);	 																// Configure the LCD A/C drive waveform.
  GLCD_Write_Data(0x0700);	 																// Line inversion 0x0700
  GLCD_Write_Command(0x03);    																// Configure Entry Mode
  GLCD_Write_Data(0x1228); 	 																// Color:RGB ,Incremen Ver.& decrement Hor. address ,Address update Ver.

  // Configure and enable the LCD power supply.
  // Power Control
  GLCD_Write_Command(0x07);    																// Enable internal operation of the LCD controller.
  GLCD_Write_Data(0x0101);
  GLCD_Write_Command(0x10);
  GLCD_Write_Data(0x0000);
  GLCD_Write_Command(0x11);
  GLCD_Write_Data(0x0007);
  GLCD_Write_Command(0x12);
  GLCD_Write_Data(0x0000);
  GLCD_Write_Command(0x13);
  GLCD_Write_Data(0x0000);
  delay_ms(20);

  GLCD_Write_Command(0x10);
  GLCD_Write_Data(0x16B0);
  GLCD_Write_Command(0x11);
  GLCD_Write_Data(0x0037);
  delay_ms(20);

  GLCD_Write_Command(0x12);
  GLCD_Write_Data(0x013E);
  delay_ms(20);

  GLCD_Write_Command(0x13);
  GLCD_Write_Data(0x1A00);
  GLCD_Write_Command(0x29);
  GLCD_Write_Data(0x000F);
  delay_ms(40);              																// Delay for 40ms while the power supply stabilizes.

  // Clear the contents of the display buffer.
  GLCD_Write_Command(0x22);
  for(cnt = 0;cnt < (320 * 240);cnt++)
  {
    GLCD_Write_Data(WHITE);    																// Back-Ground Color White
  }

  GLCD_Write_Command(0x20);     															// Command Horizontal GRAM Address Set
  GLCD_Write_Data(0x0000);    																// Address Start 0x0000
  GLCD_Write_Command(0x21);     															// Command Vertical GRAM Address Set
  GLCD_Write_Data(0x0000);    																// Address Start 0x0000

  GLCD_Write_Command(0x50);     															// Command Horizontal Address Start Position
  GLCD_Write_Data(0x0000);
  GLCD_Write_Command(0x51);     															// Command Horizontal Address End Position(239)
  GLCD_Write_Data(0x00EF);

  GLCD_Write_Command(0x52);     															// Command Vertical Address Start Position
  GLCD_Write_Data(0x0000);
  GLCD_Write_Command(0x53);     															// Command Verticall Address End Position(319)
  GLCD_Write_Data(0x013F);

  GLCD_Write_Command(0x60);     															// Set the number of lines to scan.
  GLCD_Write_Data(0x2700);

  GLCD_Write_Command(0x61);     															// Enable grayscale inversion of the source outputs.
  GLCD_Write_Data(0x0001);

   // GAMMA Control
   GLCD_Write_Command(0x30);
   GLCD_Write_Data(0x0007);
   GLCD_Write_Command(0x31);
   GLCD_Write_Data(0x0403);
   GLCD_Write_Command(0x32);
   GLCD_Write_Data(0x0404);
   GLCD_Write_Command(0x35);
   GLCD_Write_Data(0x0002);
   GLCD_Write_Command(0x36);
   GLCD_Write_Data(0x0707);
   GLCD_Write_Command(0x37);
   GLCD_Write_Data(0x0606);
   GLCD_Write_Command(0x38);
   GLCD_Write_Data(0x0106);
   GLCD_Write_Command(0x39);
   GLCD_Write_Data(0x0007);
   GLCD_Write_Command(0x3C);
   GLCD_Write_Data(0x0700);
   GLCD_Write_Command(0x3D);
   GLCD_Write_Data(0x0707);

   GLCD_Write_Command(0x07);    															//Enable the image display.
   GLCD_Write_Data(0x0173);

   GLCD_BL_ON();	 																		//Back Light ON
   delay_ms(20);	 																		//Delay for 20ms, which is equivalent to two frames.
}



/******************************************************/
/*   Function Set  Background color or Clear Screen   */
/******************************************************/
/* Parameter : bg_color =  BackGround color of Screen */
/******************************************************/
void bg_color(long bg_color)
{
  long cnt;

  GLCD_Write_Command(0x20);    																// Command Horizontal GRAM Address Set
  GLCD_Write_Data(0x0000);	   																// Address Start 0x0000
  GLCD_Write_Command(0x21);    																// Command Vertical GRAM Address Set
  GLCD_Write_Data(0x0000);	   																// Address Start 0x0000

  GLCD_Write_Command(0x22);
  for(cnt = 0;cnt < (320 * 240);cnt++)
  {
    GLCD_Write_Data(bg_color);    															// Back-Ground Color Black
  }
}

/*****************************************************/
/*              Function Plot Dot 1 Pixel            */
/*****************************************************/
/* Parameter : Xadd,Yadd = Position X,Y for Plot Dot */
/*             color = Color of dot                  */
/*****************************************************/
void plot_dot_hor(unsigned long Xadd,unsigned long Yadd,unsigned long color)
{
  GLCD_Write_Command(0x20);  																// Command Set Adddress Hor(X)
  GLCD_Write_Data(Yadd);  																	// Sent X_address for CGRAM
  GLCD_Write_Command(0x21);  																// Command Set Address Ver(Y)
  GLCD_Write_Data(Xadd);  																	// Sent Y_address for CGRAM
  GLCD_Write_Command(0x22);  																// Command Write Data RGB
  GLCD_Write_Data(color);  																	// write data
}

/****************************************************************************/
/**                Function Print Text 1 Charecter size 7x11                */
/****************************************************************************/
/* Parameter : row      = Ascii Code (Position buffer keep text)		    */
/*             adx,ady  = Position X,Y for begin plot text by will 			*/
/*                        begin plot from bottom left to top left   		*/
/*             fg_clr   = Color of text										*/
/*             bg_clr   = Color background of text(if bg_clr = no_bg or 1=	*/
/*                        non color background)								*/
/****************************************************************************/
void text_7x11_hor(unsigned char row,long adx,long ady,long fg_clr,long bg_clr)
{
  long ax,ay;
  unsigned char m,n,tx;

  ax = adx;
  ay = ady;

  row = row-0x20;

  // Print Text 1 Charecter(data 14 Byte)
  for(m=0;m<14;m++)
  {
    // Sent data byte1=8 bit
    tx = ascii_7x11[row][m];  																//Read data Ascii

	for(n=0;n<8;n++)		       															//Loop Sent data  1 byte(8bit)
	{
	  if(tx & 0x80)				   															//if data bit7 = 1 ,Plot Color area Charecter
	  {
	    GLCD_Write_Command(0x20);  															//Command Set Adddress Hor(X)
        GLCD_Write_Data(ay);  																//Sent X_Address CGRAM
        GLCD_Write_Command(0x21);  															//Command Set Address Ver(Y)
        GLCD_Write_Data(ax);  																//Sent Y_Address CGRAM
		GLCD_Write_Command(0x22);  															//Command Write data
        GLCD_Write_Data(fg_clr);
	   }
	   else						   															//if data bit7 = 0 ,Plot Color area back ground Charecter
	   {
	     if(bg_clr != 1)
         {
		    GLCD_Write_Command(0x20);  														//Command Set Adddress Hor(X)
            GLCD_Write_Data(ay);  															//Sent X_Address CGRAM
            GLCD_Write_Command(0x21);  														//Command Set Adddress Ver(Y)
            GLCD_Write_Data(ax);  															//Sent Y_Address CGRAM
			GLCD_Write_Command(0x22);  														//Command Write data
            GLCD_Write_Data(bg_clr);  														//Sent Data
		  }
		}

		tx <<= 1;  																			// Shift Right data 1 bit
		ay   = ay+1;  																		// Increment Y-address
	}
	m++;  																					//Increment Next pointter byte Data


	// Sent data byte2=3 bit
	tx = ascii_7x11[row][m];  																//Read data byte2
	for(n=0;n<3;n++)			   															//Loop sent data byte2 = 3 bit
	{
	  if(tx & 0x80)				   															//if data bit7 = 1 ,Plot Color area Charecter
	  {
	    GLCD_Write_Command(0x20);  															//Command Set Adddress Hor(X)
        GLCD_Write_Data(ay);  																//Sent X_Address CGRAM
        GLCD_Write_Command(0x21);  															//Command Set Adddress Ver(Y)
        GLCD_Write_Data(ax);  																//Sent Y_Address CGRAM
		GLCD_Write_Command(0x22);  															//Command Write data
        GLCD_Write_Data(fg_clr);
      }
	  else						   															//if data bit7 = 0 ,Plot Color area back ground Charecter
	  {
	    if(bg_clr != 1)
        {
		  GLCD_Write_Command(0x20);  														//Command Set Adddress Hor(X)
          GLCD_Write_Data(ay);  															//Sent X_Address CGRAM
          GLCD_Write_Command(0x21);  														//Command Set Adddress Ver(Y)
          GLCD_Write_Data(ax);  															//Sent Y_Address CGRAM
		  GLCD_Write_Command(0x22);  														//Command Write data
          GLCD_Write_Data(bg_clr);
		}
	  }

	  tx <<= 1;  																			//Shift Right data 1 bit
	  ay = ay+1;  																			//Increment Y-address
	}

	ax = ax+1; 																				//Complet sent data 2 byte(11bit) Increment X-Address
	ay = ady; 																				//Set Position Y-address old
  }

  // Fill Back ground Color Position space between Charecter 1 Colum
  if(bg_clr != 1)
  {
    for(n=0;n<11;n++)
	{
	  GLCD_Write_Command(0x20);  															//Command Set Adddress Hor(X)
      GLCD_Write_Data(ay);  																//Sent X_Address CGRAM
      GLCD_Write_Command(0x21);  															//Command Set Adddress Ver(Y)
      GLCD_Write_Data(ax);  																//Sent Y_Address CGRAM
	  GLCD_Write_Command(0x22);  															//Command Write data
      GLCD_Write_Data(bg_clr);
	  ay = ay+1;  																			//Increment Y-Address
    }
  }
}

/**********************************************************************/
/*                         Function Print String                      */
/**********************************************************************/
/* Parameter : *str = Charecter ASCII (String)					      */
/*             cur_x,cur_y = Position X,Y for begin plot text by will */
/*                           begin plot from bottom left to top left  */
/*             fg_color = color of Text								  */
/*             bg_color = color Background of text                    */
/**********************************************************************/
void lcd_printStr_hor(char *str,long cur_x,long cur_y,long fg_color,long bg_color)
{
  unsigned char i;

  for (i=0; str[i] != '\0'; i++)
  {
    text_7x11_hor(str[i],cur_x,cur_y,fg_color,bg_color);
	cur_x += 8;
  }
}


/********************************************************/
/*               Function Draw Line 		     		*/
/********************************************************/
/* Function : Line(x_1,y_1,x_2,y_2,line_type);	        */
/*												     	*/
/* Input  : x_1,y_1    = Start Coordinate Line			*/
/*          x_2,y_2    = End Coordinate Line			*/
/*          line_type  = Value Thick of Line (1-100)    */
/*	        color      = Color of line			        */
/********************************************************/
void Line(long x_1,long y_1,long x_2,long y_2,char line_type,long color)
{
  char tick;
  long x1=x_1,xx1=x_1;
  long y1=y_1;
  long x2=x_2,xx2=x_2;
  long y2=y_2;
  long dx,dy,inc_dec,two_dx,two_dy,two_dy_dx,two_dx_dy,x,y,p;

  if(x_1>x_2)
  {
    x1=x_2;
	y1=y_2;

	x2=x_1;
	y2=y_1;
  }

  dx  = fabs(x2-x1);
  dy  = fabs(y2-y1);

  if(y2>=y1)
  inc_dec = 1;
  else
  inc_dec = -1;

  if(dx>dy)
  {
    two_dy=(2*dy);
	two_dy_dx=(2*(dy-dx));
	p=((2*dy)-dx);

	x=x1;
	y=y1;

	while(x<=x2)
	{
	  for(tick=0;tick<line_type;tick++)
	  {
	    if(xx1==xx2)
		plot_dot_hor(x+tick,y,color);
		else
		plot_dot_hor(x,y+tick,color);
	  }

	  x++;
	  if(p<0)
	  p += two_dy;
	  else
	  {
	    y += inc_dec;
		p += two_dy_dx;
	  }
	}
  }

  else
  {
    two_dx=(2*dx);
	two_dx_dy=(2*(dx-dy));
	p=((2*dx)-dy);

	x=x1;
	y=y1;

	while(y!=y2)
	{
	  for(tick=0;tick<line_type;tick++)
	  {
	    if(xx1==xx2)
		plot_dot_hor(x+tick,y,color);
		else
		plot_dot_hor(x,y+tick,color);
	  }
	  y+=inc_dec;

	  if(p<0)
	  p+=two_dx;

	  else
	  {
	    x++;
		p += two_dx_dy;
	  }
	}
  }
}

/************************************************************************************/
/*                          Function Draw a rectangle 			                    */
/************************************************************************************/
/* Function : Rectan(x1,y1,x2,y2,tick,color,fill)									*/
/* Inputs   : (x1, y1)    = Start Point coordinate Rectan							*/
/*            (x2, y2)    = End Point coordinate  Rectan							*/
/*            tick(1-100) = bold of line(have effect when fill=0					*/
/*            color       =  color of rectangle									    */
/*            fill        = 0: No fill color in rectangle,1:Fill color in rectangle */
/*       																		    */
/*            (x1,y1) -------------												    */
/*                   |             |											    */
/*                   |             |											    */
/*                   |             |											    */
/*                   |             |											    */
/*                    -------------	(x2,y2)										    */
/*																					*/
/************************************************************************************/
void Rectan(long x1,long y1,long x2,long y2,char tick,long color,char fill)
{
  long xmin,xmax,ymin,ymax,i;
  char tk = tick-1;

  if(fill==1)
  {
    if(x1 < x2)                            													//  Find x min and max
    {
      xmin = x1;
      xmax = x2;
	}
	else
	{
	  xmin = x2;
	  xmax = x1;
	}
	if(y1 < y2)                            													// Find the y min and max
	{
	  ymin = y1;
	  ymax = y2;
	}
	else
	{
	  ymin = y2;
	  ymax = y1;
	}

	for(; xmin<=xmax;xmin++)
	{
	  for(i=ymin; i<=ymax; i++)
	  plot_dot_hor(xmin,i,color);
	}
  }
  else
  {
    Line(x1,y1-tk,x2,y1-tk,tick,color);  													//Line Ho.r Top
	Line(x1,y2,x2,y2,tick,color);  															//Line Hor. Botton
	Line(x1,y1,x1,y2,tick,color);  															//Line Ver. Left
	Line(x2-tk,y1,x2-tk,y2,tick,color);  													//Line Ver. Right
  }
}


/*********************************************************************/
/*         Function Plot Picture(BMP) to Display                     */
/*  Start Plot data of picture from  top left to top Right           */
/*********************************************************************/
/*  Inputs : *bmp       = Name reg. array keep data of picture       */
/*           xad,yad    = Position X,Y for begin plot data picture   */
/*           width,high = Size Picture unit Pixel                    */
/*********************************************************************/
void plot_picture_hor(const char *bmp,long xad,long yad,long width,long high)
{
  unsigned long pt=0,picture;
  unsigned long hi,wid;
  long xad1,yad1;

  xad1 = xad;
  yad1 = yad;

  for(hi=0;hi<high;hi++)		        													// Number X-Pixel(High) of Picture
  {
    GLCD_Write_Command(0x20);  																// Command Set Address Hor(X)
    GLCD_Write_Data(yad1);  																// Sent X_Address CGRAM

	for(wid=0;wid<width;wid++)	        													// number X-pixel(Width) of Picture
	{
	  GLCD_Write_Command(0x21);  															// Command Set Adddress Ver(Y)
      GLCD_Write_Data(xad1);  																// Sent Y_Address CGRAM

	  GLCD_Write_Command(0x22);  															// Command Write data

	  picture = bmp[pt+1];  																// Read data picture byte High
	  picture =	picture <<8;

	  picture|= bmp[pt];  																	// read data picture Byte low

      GLCD_Write_Data(picture);
	  pt = pt+2;  																			// Increment pointer of data picture 2 byte

	  xad1++;
	}

	yad1--;
	xad1 = xad;
  }
}


// GUI FUNCTIONS


