#include "runtime.h"
#include "stdlib.h"
#include "stdio.h"

void mp_raise_msg(const int *exc_type, const char * msg) {
    printf("Exception raised: %s\n", msg);
    exit(1);
}

void mp_raise_msg_varg(const int *exc_type, const char * fmt, ...) {
    printf("Exception raised\n");
    exit(1);
}

void *mp_obj_new_exception_msg(const void *exc_type, const char *msg)
{
    return NULL;
}

void nlr_jump(void *val)
{
    printf("nlr_jump\n");
    while (1);
}