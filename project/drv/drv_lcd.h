#ifndef DRV_LCD
#define DRV_LCD

/* Font Table */
extern const char ascii_7x11[95][14]; 	// font.c

#define GLCD_BL_ON()	LPC_GPIO4->FIOPIN |=  (1UL << 28); 									// P4.28(BL=High:ON)
#define	GLCD_BL_OFF()   LPC_GPIO4->FIOPIN &= ~(1UL << 28); 									// P4.28(BL=Low:OFF)

/******************************/
/* Define Color Code For GLCD */
/* Color = RRRRR GGGGGG BBBBB */
/******************************/
#define no_bg           0x0001			// No Color Back Ground
#define BLACK           0x0000
#define WHITE           0xFFFF
#define RED             0x8000
#define GREEN           0x0400
#define BLUE            0x0010
#define YELLOW          0xFFF0
#define CYAN            0x0410
#define MAGENTA         0x8010
#define BROWN           0xFC00
#define OLIVE	        0x8400

#define BRIGHT_RED      0xF800
#define BRIGHT_GREEN    0x07E0
#define BRIGHT_BLUE     0x001F
#define BRIGHT_YELLOW   0xFFE0
#define BRIGHT_CYAN     0x07FF
#define BRIGHT_MAGENTA  0xF81F

#define LIGHT_GRAY      0x8410
#define DARK_GRAY       0x4208
#define LIGHT_BLUE      0x841F
#define LIGHT_GREEN     0x87F0
#define LIGHT_CYAN      0x87FF
#define LIGHT_RED       0xFC10
#define LIGHT_MAGENTA   0xFC1F

#define GRAY0       	0xE71C
#define GRAY1         	0xC618
#define GRAY2           0xA514
#define GRAY3           0x630C
#define GRAY4           0x4208
#define GRAY5	        0x2104
#define	GRAY6	        0x3186

#define	BLUE0	        0x1086
#define	BLUE1       	0x3188
#define	BLUE2          	0x4314
#define BLUE3         	0x861C

#define	CYAN0	        0x3D34
#define	CYAN1           0x1DF7

#define	GREEN0          0x0200
#define	GREEN1          0x0208

//------------ Color For Build Button -------------
#define BT_RED          1
#define BT_GREEN        2
#define BT_BLUE         3
#define	BT_YELLOW       4
#define BT_WHITE        5
#define BT_CYAN         6
#define BT_MAGENTA      7
#define BT_BROWN        8

#define BT_LRED         9
#define BT_LGREEN       10
#define BT_LBLUE        11
#define	BT_LYELLOW      12
#define BT_LWHITE       13
#define BT_LCYAN        14
#define BT_LMAGENTA     15
#define BT_LBROWN       16

void drv_lcd_load				(void);
void drv_lcd_open				(void);

void drv_lcd_close				(void);
void drv_lcd_unload				(void);

/* Function Application of GLCD */
void drv_lcd_bg_color			(long bg_color);
void drv_lcd_plot_dot_hor		(unsigned long Xadd,unsigned long Yadd,unsigned long color);
void drv_lcd_text_7x11_hor		(unsigned char row,long adx,long ady,long fg_clr,long bg_clr);
void drv_lcd_printStr_hor		(char *str,long cur_x,long cur_y, long fg_color,long bg_color);
void drv_lcd_line				(long x_1,long y_1,long x_2,long y_2,char line_type,long color);
void drv_lcd_rectan				(long x1,long y1,long x2,long y2,char tick,long color,char fill);
void drv_lcd_plot_picture_hor	(const char *bmp,long xad,long yad,long width,long high);
void drv_lcd_plot_mark_hor		(long x1,long y1,long x2,long y2,long color);
void drv_lcd_lcd_print3Cha_hor	(char ch1,char ch2,char ch3,long cur_x,long cur_y,long fg_color,long bg_color);

#endif //DRV_LCD
