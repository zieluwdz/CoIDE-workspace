#ifndef DRV_SD
#define DRV_SD

#include "stdint.h"

void drv_sd_load	(void);
void drv_sd_open	(uint32_t maxMessageNumber);

void drv_sd_close	(void);
void drv_sd_unload	(void);

void drv_sd_write(char *pMessageData, uint32_t messageSize);

#endif //DRV_SD
