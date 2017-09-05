#include "drv_sd.h"
#include "LPC17xx.h"

#define	DRV_SD_CS_HIGH()	LPC_GPIO0->FIOPIN |=  (1UL<<16);	// P0.16(CS=High:Disable)
#define	DRV_SD_CS_LOW()		LPC_GPIO0->FIOPIN &= ~(1UL<<16); 	// P0.16(CS=Low:Enable)

static unsigned char drv_sd_spi_write(unsigned char DataByte);

void drv_sd_load(void)
{
	//Config P0.15,P0.16,P0.17,P0.18 = SPI Interface
	LPC_SC->PCONP |= (1 << 8); //Enable power to SPI

	LPC_SC->PCLKSEL0 &= ~(3<<16);	//PCLK_SPI=CCLK/4(18MHz)
	LPC_SC->PCLKSEL0 |= (2<<16);	//PCLK_SPI=CCLK/2(100MHz / 2)
	LPC_SPI->SPCCR = 180; 			//50MHz/180=400kBit// SSEL is GPIO, output set to high.

	LPC_PINCON->PINSEL1 &= ~(3UL<<0); 					// Reset P0.16 Mode = GPIO
	LPC_GPIO0->FIODIR   |=  (1UL<<0);	 				// P0.16 = CS(Output)
	LPC_GPIO0->FIOPIN   |=  (1UL<<0);    				// P0.16 = High

	//Config SPI Pin Connect
	LPC_PINCON->PINSEL0 &= ~(3UL<<30); 	//P0.15 cleared
	LPC_PINCON->PINSEL0 |= (3UL<<30); 	//P0.15 SCK
	LPC_PINCON->PINSEL1 &= ~((3<<2)|(3<<4)); //P0.17, P0.18cleared
	LPC_PINCON->PINSEL1 |= ((3<<2)|(3<<4)); //P0.17 MISO,P0.18MOSI

	//Config SPI = Master,8Bit,CPOL=0,CPHA=0
	LPC_SPI->SPCR &= ~(1<<3);	//CPHA = 0 RisingClock
	LPC_SPI->SPCR &= ~(1<<4); 	//CPOL = 0
	LPC_SPI->SPCR |= (1<<5); 	//MSTR = 1 = MasterSPI
	LPC_SPI->SPCR &= ~(1<<6); 	//LSBF = 0 = MSB First

	LPC_SPI->SPCR &= ~(1<<7); //SPIE = 0 = Disable INT
	LPC_SPI->SPCR &= ~(15<<8); //BIT = 0000(Bits Format)
	LPC_SPI->SPCR |= (1<<11); //BIT = 1000(8 Bit Data)
	LPC_SPI->SPCR |= (1<<2); //Enable SPI

	DRV_SD_CS_HIGH(); //CS High

	int i = 0;
	for(i=0; i<80u;i++)
	{
		drv_sd_spi_write(0xffu);
	}

	uint8_t test = 0;
	do
	{
	DRV_SD_CS_LOW();
	test = drv_sd_spi_write(0x40);
	test = drv_sd_spi_write(0x00);
	test = drv_sd_spi_write(0x00);
	test = drv_sd_spi_write(0x00);
	test = drv_sd_spi_write(0x00);
	test = drv_sd_spi_write(0x95);
	DRV_SD_CS_HIGH()
	}
	while(test == 255);

	DRV_SD_CS_HIGH()
}

void drv_sd_open(void)
{

}

void drv_sd_close(void)
{

}

void drv_sd_unload(void)
{

}

static unsigned char drv_sd_spi_write(unsigned char DataByte)
{
  LPC_SPI->SPDR = DataByte; 		// Write and Read a byte on SPI interface.
  while (!(LPC_SPI->SPSR & 0x80));     // Wait BYS for transfer to finish
  return (LPC_SPI->SPDR);          // Return received value
}
