#ifndef __RUNTIME_H
#define __RUNTIME_H

#include <stdint.h>
#include <stdbool.h>

#if __cplusplus
extern "C" {
#endif

#define MP_ERROR_TEXT(s)        (s)
#define MP_STACK_CHECK()
void mp_raise_msg(const int *exc_type, const char * msg);
void mp_raise_msg_varg(const int *exc_type, const char * fmt, ...);
void *mp_obj_new_exception_msg(const void *exc_type, const char *msg);

#if __cplusplus
}
#endif

#endif // __RUNTIME_H