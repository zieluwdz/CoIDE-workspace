#ifndef DRV_USRINT
#define DRV_USRINT

#include <stdint.h>

typedef void (*event_cb_t)(void);

void drv_usr_int_load	(void);
void drv_usr_int_unload	(void);

void drv_usr_int_open	(event_cb_t pfCallback);
void drv_usr_int_close	(void);

#endif //DRV_USRINT
