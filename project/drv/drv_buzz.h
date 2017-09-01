#ifndef DRV_BUZZ
#define DRV_BUZZ

#include <stdint.h>

void drv_buzz_load		(void);
void drv_buzz_open		(void);

void drv_buzz_on		(void);
void drv_buzz_off		(void);

void drv_buzz_generateStart	(uint32_t delay_us);
void drv_buzz_generateStop	(void);

void drv_buzz_close		(void);
void drv_buzz_unload	(void);

#endif //DRV_BUZZ
