#ifndef DRV_TCS
#define DRV_TCS

void drv_tcs_load	(void);
void drv_tcs_open	(void);

void drv_tcs_close	(void);
void drv_tcs_unload	(void);

void drv_tcs_calibrate(void);
char Get_Status_Touch(void);
void TCS_Get_Point(char num);

#endif //DRV_TCS
