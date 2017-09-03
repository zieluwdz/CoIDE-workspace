#include "drv_tcs.h"
#include "LPC17xx.h"
#include "drv_lcd.h" //need for calibration

#define	TCS_CS_HIGH()	LPC_GPIO1->FIOPIN |=  (1UL<<21);    								// P1.21(CS=High:Disable)
#define	TCS_CS_LOW()    LPC_GPIO1->FIOPIN &= ~(1UL<<21);   									// P1.21(CS=Low:Enable)

#define PENIRQ_MASK     0x20 																// 00000000 00100000 00000000 00000000 = P0.21 => 8Bit Mask(00100000)
#define PENIRQ_READ()  (LPC_GPIO0->FIOPIN >> 16) & PENIRQ_MASK;   							// 00000000 00100000 00000000 00000000 = P0.21 => 8Bit Mask(00100000)


// Variable for Touch Screen Function GLCD
static uint32_t dif_adc_X,dif_adc_Y;
static uint32_t buf_adc_X[100],buf_adc_Y[100];
static uint32_t ave_adc_X,ave_adc_Y;
static uint32_t tcs_ave_X[3];																			// Keep Value adc Touch Point X
static uint32_t tcs_ave_Y[3];																			// Keep Value adc Touch Point Y
static uint32_t dis_XD_hor[3] = {32,287,160}; 															// Value refer Point X at 10% of Display(X=320:0-319) 3 Position
static uint32_t dis_YD_hor[3] = {215,120,24}; 															// Value refer Point Y at 10% of Display(Y=240:0-239) 3 Position
static uint32_t divider,An,Bn,Cn,Dn,En,Fn,X,Y;															// Valiable for keep coefficient Calibrat and position touch Screen
static unsigned char num1,num2,num3;																	// Variable for keep data Dec to Ascii

static unsigned char TCS_SPI_Write(unsigned char DataByte);
static void TCS_SPI_Read_Hor(void);
static void TCS_Average_X_Y(unsigned char num);
static void TCS_Set_Matrix_Hor(void);

void drv_tcs_load	(void)
{
	  LPC_PINCON->PINSEL3 &= ~(3UL<<10); 					// Reset P1.21 Mode = GPIO
	  LPC_GPIO1->FIODIR   |=  (1UL<<21);	 				// P1.21 = ADS7846 CS(Output)
	  LPC_GPIO1->FIOPIN   |=  (1UL<<21);    				// P1.21 = High

	  LPC_PINCON->PINSEL1 &= ~(3UL<<10); 					// Reset P0.21 Mode = GPIO
	  LPC_GPIO0->FIODIR   &= ~(1UL<<21);					// P0.21 = PENIRQ(Input)

	  //Config SSP0 Pin Connect
	  LPC_PINCON->PINSEL3 |=  (3UL<<8);         			// Select P1.20 = SCK0(SSP0)
	  LPC_PINCON->PINSEL3 |=  (3UL<<14);        			// Select P1.23 = MISO0(SSP0)
	  LPC_PINCON->PINSEL3 |=  (3UL<<16);        			// Select P1.24 = MOSI0(SSP0)

	  LPC_SC->PCONP       |=  (1 << 21);        			// Enable power to SSPI0 block
	  LPC_SC->PCLKSEL1    &= ~(3<<10);          			// PCLKSP0 = CCLK/4 (18MHz)
	  LPC_SC->PCLKSEL1    |=  (1<<10);          			// PCLKSP0 = CCLK   (72MHz)
	  LPC_SSP0->CPSR       =   72;              			// 72MHz / 72 = 1MHz(maximum of 2MHz is possible)

	  LPC_SSP0->CR0        =  (   0 << 7) | 				// CPHA = 0
	                          (   0 << 6) | 				// CPOL = 0
	                          (   0 << 4) | 				// Frame format = SPI
	                          ((8-1)<< 0) ; 				// Data size = 8 bits
	  LPC_SSP0->CR1        =  (   1 << 1);  				// Enable SSP-
}

void drv_tcs_open	(void)
{

}

void drv_tcs_close	(void)
{

}

void drv_tcs_unload	(void)
{

}

/************************************************************/
	/*              Function Calibrate Touch Green              */
	/* Touch at Point + on Screen 3 Point(3 step) for Caribrate */
	/* Parameter    : None                                      */
	/* Return value : None                                      */
	/*              											*/
	/************************************************************/
void drv_tcs_calibrate(void)
{
	  unsigned char n,touch_mark_point=0;

	  plot_mark_hor(25,215,32,222,BRIGHT_RED);                                  				// Plot Signe + for Position Calibrate Touch Point1
	  lcd_printStr_hor("<Touch Screen Calibratetion>",45,164,BRIGHT_BLUE,BLACK);   				// Plot Text	Color Bright blue
	  lcd_printStr_hor("        STEP 1 of 3         ",45,144,BRIGHT_BLUE,BLACK);
	  lcd_printStr_hor(" Click on center of '+' icon",45,104,WHITE,BLACK);        				// Plot Text	Color Black
	  lcd_printStr_hor(" for Touch screen calibrate ",45,84,WHITE,BLACK);

	  while(touch_mark_point != 3)	        													// Number Mark Point Calibrate = 3 Point
	  {
	    if(Get_Status_Touch())                 													// return (1) if touch screen is detected.
	    {
		  for(n=0;n<50;n++)
		  {
		    TCS_SPI_Read_Hor();            														// Read portrait(ADC X,Y) by touch value from ADS7846 and update dif_adc_X & dif_adc_Y
			buf_adc_X[n] = dif_adc_X;  															// General buffer for averaging and filter touch screen adc read outs.
		    buf_adc_Y[n] = dif_adc_Y;  															// General buffer for averaging and filter touch screen adc read outs.
		  }
		  TCS_Average_X_Y(50);             														// Take average out of 50 touch measurements, excluding max and min measurement.

		  tcs_ave_X[touch_mark_point] =  ave_adc_X;
		  tcs_ave_Y[touch_mark_point] =  ave_adc_Y;

	      touch_mark_point++;            														// touch_cal_status : 0=xy1 cal, 1=xy2 cal, 2=xy3 cal, 3=calibration success. 0xff=calibration not performed.

		  while(Get_Status_Touch());        													// Waite until not Touch screen
	      switch(touch_mark_point)																// Select Caribrate Next Step 2 and 3
		  {
		    case 1: bg_color(BLACK);                                                        	// Clear Screen
					plot_mark_hor(280,120,287,127,BRIGHT_RED);                                  // Plot Signe + for Position Calibrate Touch Point2

	                lcd_printStr_hor("<Touch Screen Calibratetion>",45,164,BRIGHT_BLUE,BLACK);  // Plot Text Color Bright blue
	                lcd_printStr_hor("        STEP 2 of 3         ",45,144,BRIGHT_BLUE,BLACK);
	                lcd_printStr_hor(" Click on center of '+' icon",45,104,WHITE,BLACK);        // Plot Text Color white
	                lcd_printStr_hor(" for Touch screen calibrate ",45,84,WHITE,BLACK);
			        break;

			case 2:	bg_color(BLACK);                                                         	//Clear Screen
			        plot_mark_hor(153,24,160,31,BRIGHT_RED);                                    //Plot Signe + for Position Calibrate Touch Point3

					lcd_printStr_hor("<Touch Screen Calibratetion>",45,164,BRIGHT_BLUE,BLACK);  //Plot Text	Color Bright blue
	                lcd_printStr_hor("        STEP 3 of 3         ",45,144,BRIGHT_BLUE,BLACK);
	                lcd_printStr_hor(" Click on center of '+' icon",45,104,WHITE,BLACK);        //Plot Text	Color white
	                lcd_printStr_hor(" for Touch screen calibrate ",45,84,WHITE,BLACK);
					break;

			case 3: bg_color(BLACK);                                                         	//Clear Screen

					lcd_printStr_hor("<Touch Screen Calibratetion>",45,159,BRIGHT_BLUE,BLACK);  //Plot Text	Color Bright blue
	                lcd_printStr_hor("          Succeded!         ",45,119,BRIGHT_GREEN,BLACK);
	                lcd_printStr_hor("   -Tap Screen for exit-    ",45,79,WHITE,BLACK);

					while(!Get_Status_Touch());
					delay_ms(100);
					bg_color(BLACK);                                                         	// Clear Screen
		            break;
		  }
		}
	    delay_ms(100);
	  }
	  TCS_Set_Matrix_Hor(); 																	// Calculate Coefficient for use find Position Address real on Display

	  //You can Keep Value data in valiabie devider,An,Bn,Cn,Dn,En,Fn to E2Prom  so that not
	  //come back	 Caribrate.
	/*
	  printf("\n\r divider = %d",divider) ;
	  printf("\n\r      An = %d",An) ;
	  printf("\n\r      Bn = %d",Bn) ;
	  printf("\n\r      Cn = %d",Cn) ;
	  printf("\n\r      Dn = %d",Dn) ;
	  printf("\n\r      En = %d",En) ;
	  printf("\n\r      Fn = %d",Fn) ;
	*/
}

/***************************************************************************/
/*    Function SPI Write and Read Data 1 Byte from Touch Screen ADS7846    */
/***************************************************************************/
/* Parameter    : DataByte = data or command control ADS7846 (1Byte)       */
/* Return Value : Return value adc from touched times 1 byte Pass Function */
/***************************************************************************/
static unsigned char TCS_SPI_Write(unsigned char DataByte)
{
  LPC_SSP0->DR = DataByte; 																	// Write and Read a byte on SPI interface.
  while (LPC_SSP0->SR & 0x10);          													// Wait BYS for transfer to finish
  return (LPC_SSP0->DR);                													// Return received value
}

/*****************************************************************/
/* Function Read X-Y-Position ADC Touch Screen-12 bit (ADS7846)  */
/* Parameter    : None                                           */
/* Return Value : dif_adc_X = Keep Result ADC X-Position(12 bit) */
/*                dif_adc_Y = Keep result ADC Y-Position(12 bit) */
/*****************************************************************/
static void TCS_SPI_Read_Hor(void)
{
  long tcs_adc_X ,tcs_adc_Y;
  unsigned char buf_data[4];

  if(Get_Status_Touch())
  {
    TCS_CS_LOW();																			// Enable Touch Interface

    buf_data[0] = TCS_SPI_Write(0xD0);														// Write Command Measure X-Position
	buf_data[0] = TCS_SPI_Write(0x00);														// Read ADC data X-Position (7-bit byte High) data: 0ddddddd	(bit)
	buf_data[1] = TCS_SPI_Write(0x90);														// Write Command Measure Y-Position ; Read ADC data X-Position (5-bit byte Low)  data:ddddd000(bit)
	buf_data[2] = TCS_SPI_Write(0x00);														// Read ADC data Y-Position(7-bit byte High) data: 0ddddddd (bit)
	buf_data[3] = TCS_SPI_Write(0x00);														// Read ADC data Y-Position(5-bit byte Low)  data: ddddd000 (bit)

	tcs_adc_X  = buf_data[0];																// Mark ADC Data X-Position 12 bit
	tcs_adc_X  = tcs_adc_X << 5;
	tcs_adc_X |= buf_data[1] >> 3;
	tcs_adc_X  = tcs_adc_X & 0x00000FFF;

	tcs_adc_Y  = buf_data[2];   															// Mark ADC Data Y-Position 12 bit
	tcs_adc_Y  = tcs_adc_Y << 5;   															// Shift 7 bit High
	tcs_adc_Y |= buf_data[3] >> 3;   														// Shift 5 bit low
	tcs_adc_Y  = tcs_adc_Y & 0x00000FFF;													// total ADC data 12 bit

	TCS_CS_HIGH();																			// Disable Touch Interface

    //Result
    dif_adc_X = 4095-tcs_adc_Y;  															// ADC 12 bit :LCD Start Landscape
	dif_adc_Y = tcs_adc_X;
  }
}


/************************************************************************************/
/* Function Calculate average of measurements,by cut value max and min measurement. */
/************************************************************************************/
/* Parameter    : num = number times of measurement adc from touch screen           */
/* Return value : ave_adc_X = Keep result average adc X-Position			        */
/*                ave_adc_Y = Keep result average adc Y-Position                    */
/************************************************************************************/
static void TCS_Average_X_Y(unsigned char num)
{
  unsigned char i;
  uint32_t sum_x, sum_y;
  uint32_t min_x, min_y, max_x, max_y;

  sum_x=0;sum_y=0;min_x=0xffff;min_y=0xffff,max_x=0;max_y=0;
  for(i=0;i<num;i++)
  {
    sum_x += buf_adc_X[i];
	sum_y += buf_adc_Y[i];

	if(buf_adc_X[i]< min_x)		                        									// search for minimum X value
	{
	  min_x = buf_adc_X[i];
    }
	else if (buf_adc_X[i]> max_x)	                    									// search for maximum X value
	{
	  max_x = buf_adc_X[i];
	}

    if(buf_adc_Y[i]< min_y)	  	                        									// search for minimum Y value
	{
	  min_y = buf_adc_Y[i];
	}
	else if(buf_adc_Y[i]> max_y)	  	                									// search for maximum Y value
	{
	  max_y = buf_adc_Y[i];
	}
  }
  ave_adc_X = (sum_x - min_x - max_x ) / (num-2); 											// update x average value
  ave_adc_Y = (sum_y - min_y - max_y ) / (num-2);	    									// update y average value
}

/************************************************************/
/*     Function Calculate Matrix Find value Coeffcient      */
/************************************************************/
/* Parameter    : None                                      */
/* Return Value : Keep value Coeffcient for use calculate   */
/*                X-Y-Position in variable divider,An,Bn,Cn */
/*                Dn,En,Fn                                  */
/************************************************************/
static void TCS_Set_Matrix_Hor()
{
  divider = ((tcs_ave_X[0]-tcs_ave_X[2])*(tcs_ave_Y[1]-tcs_ave_Y[2]))-
	        ((tcs_ave_X[1]-tcs_ave_X[2])*(tcs_ave_Y[0]-tcs_ave_Y[2]));

       An = ((dis_XD_hor[0]-dis_XD_hor[2])*(tcs_ave_Y[1]-tcs_ave_Y[2]))-
			((dis_XD_hor[1]-dis_XD_hor[2])*(tcs_ave_Y[0]-tcs_ave_Y[2]));

	   Bn = ((tcs_ave_X[0]-tcs_ave_X[2])*(dis_XD_hor[1]-dis_XD_hor[2]))-
				((dis_XD_hor[0]-dis_XD_hor[2])*(tcs_ave_X[1]-tcs_ave_X[2]));

	   Cn = (tcs_ave_X[2]*dis_XD_hor[1]-tcs_ave_X[1]*dis_XD_hor[2])*tcs_ave_Y[0]+
		    (tcs_ave_X[0]*dis_XD_hor[2]-tcs_ave_X[2]*dis_XD_hor[0])*tcs_ave_Y[1]+
			(tcs_ave_X[1]*dis_XD_hor[0]-tcs_ave_X[0]*dis_XD_hor[1])*tcs_ave_Y[2];

	   Dn = ((dis_YD_hor[0]-dis_YD_hor[2])*(tcs_ave_Y[1]-tcs_ave_Y[2]))-
			((dis_YD_hor[1]-dis_YD_hor[2])*(tcs_ave_Y[0]-tcs_ave_Y[2]));

	   En = ((tcs_ave_X[0]-tcs_ave_X[2])*(dis_YD_hor[1]-dis_YD_hor[2]))-
			((dis_YD_hor[0]-dis_YD_hor[2])*(tcs_ave_X[1]-tcs_ave_X[2]));

	   Fn = (tcs_ave_X[2]*dis_YD_hor[1]-tcs_ave_X[1]*dis_YD_hor[2])*tcs_ave_Y[0]+
		    (tcs_ave_X[0]*dis_YD_hor[2]-tcs_ave_X[2]*dis_YD_hor[0])*tcs_ave_Y[1]+
			(tcs_ave_X[1]*dis_YD_hor[0]-tcs_ave_X[0]*dis_YD_hor[1])*tcs_ave_Y[2];
}

/*******************************************************************/
/*          Function Get Position Address real of Display          */
/*******************************************************************/
/* Parameter    : num = Number times for Measure adc from Touched  */
/* Return Value : X = Keep value address X-Position		           */
/*                Y = Keep value address Y-Position                */
/*******************************************************************/
void TCS_Get_Point(char num)
{
  unsigned char nm;

  nm=0;
  while(Get_Status_Touch() && nm<num)	  													// Measure touch x,y 10 times if the Touch kept pressed,
  {
    TCS_SPI_Read_Hor();                     												// Read value ADC Touch X-Y
	buf_adc_X[nm]=	dif_adc_X; 																// keep value ADC Touch-X
	buf_adc_Y[nm]=	dif_adc_Y; 																// keep value ADC Touch-Y

 	nm++;
  }

  if(nm==num)	                      														// if the touch xy successfuly collected,
  {
    TCS_Average_X_Y(num); 																	// take average out of 10 touch measurements, excluding max and min measurement.

    dif_adc_X = ave_adc_X; 																	// keep average ADC Touch-X
	dif_adc_Y = ave_adc_Y; 																	// keep average ADC Touch-Y

	X = ((An*dif_adc_X)+(Bn*dif_adc_Y)+Cn)/divider;
	Y = ((Dn*dif_adc_X)+(En*dif_adc_Y)+Fn)/divider;
  }
}

/********************************************/
/* Function Check Status Press Touch Screen */
/* Return Value : 0 = Not Touched screen    */
/*                1 = Touched screen        */
/********************************************/
char Get_Status_Touch(void)
{
  uint32_t pen_val;

  pen_val = PENIRQ_READ();																	// Read PENIRQ Pin Logic
  if(pen_val == PENIRQ_MASK)																// Logic "1" = Not Press
  	return 0 ;			  																	// Not Press
  else
    return 1 ;			  																	// Press Touch screen
}
