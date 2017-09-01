#ifndef DRV_POT
#define DRV_POT

#include <stdint.h>

void drv_pot_load	(void);
void drv_pot_unload	(void);

void drv_pot_open	(void);
void drv_pot_close	(void);

uint32_t drv_pot_getValue	(void);

#endif //DRV_POT
