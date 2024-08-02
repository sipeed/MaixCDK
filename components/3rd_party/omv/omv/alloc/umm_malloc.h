/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2007-2017 Ralph Hempel
 * Copyright (c) 2017-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2017-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * UMM memory allocator.
 */
#ifndef __UMM_MALLOC_H__
#define __UMM_MALLOC_H__

#include "xalloc.h"
#include <stdio.h>

static inline void umm_alloc_fail()
{
    printf("UMM: alloc failed\n");
    exit(1);
}

static inline void umm_init_x(size_t size)
{
    (void)size;
}

static inline void *umm_malloc(size_t size)
{
    return xalloc(size);
}

static inline void *umm_calloc(size_t num, size_t size)
{
    return xcalloc(num, size);
}

static inline void *umm_realloc(void *ptr, size_t size)
{
    return xrealloc(ptr, size);
}

static inline void umm_free(void *ptr)
{
    xfree(ptr);
}
#endif /* __UMM_MALLOC_H__ */
