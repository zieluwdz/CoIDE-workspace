#include "mdw_error.h"

void mdw_log_error(const char *pFunctionName_nt, size_t functionSize, const size_t line,const char *pLineMessage, size_t lineMessageSize)
{
	__asm__("BKPT");
	while(1u)
	{}
}
