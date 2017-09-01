#ifndef DRV_LED
#define DRV_LED

#include <stdint.h>

typedef enum
{
	k_OneWave	= 0x00u,
	k_Wave 		= 0x01u,
}drv_led_mode;

void drv_led_load		(void);
void drv_led_open		(void);

void drv_led_start		(drv_led_mode mode, uint32_t delay_ms);
void drv_led_speed		(uint32_t delay_ms);
void drv_led_stop		(void);

void drv_led_set		(uint8_t valueLed);
void drv_led_clear		(uint8_t valueLed);

void drv_led_setOne		(uint8_t singleLed);
void drv_led_clearOne	(uint8_t singleLed);

void drv_led_close		(void);
void drv_led_unload		(void);

#endif //DRV_LED
