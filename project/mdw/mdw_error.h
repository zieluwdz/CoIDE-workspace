#ifndef MDW_ERROR
#define MDW_ERROR

#include <stddef.h>

#define MDW_LOG_ERROR(msg) do{mdw_log_error(__func__, sizeof(__func__) - 1u, __LINE__, msg, sizeof(msg) - 1u);}while(0u != 0u)

void mdw_log_error(const char *pFunctionName_nt, size_t functionSize, const size_t line,const char *pLineMessage, size_t lineMessageSize);

#endif //MDW_LOG_DEBUG
