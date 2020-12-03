#ifndef DRV_TCS
#define DRV_TCS

#include <stdint.h>

void drv_tcs_load	(void);
void drv_tcs_open	(void);
void drv_tcs_close	(void);
void drv_tcs_unload	(void);

char drv_tcs_get_status_touch	(void);
void drv_tcs_get_point			(char num, uint64_t *pAdcTouchX, uint64_t *pAdcTouchY);

//DO NOT USE CALIBRATATION. You have to modify the code before :)
void drv_tcs_calibrate			(void);


#endif //DRV_TCS
