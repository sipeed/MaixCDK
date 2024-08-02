/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Memory allocation functions.
 */
#ifndef __XALLOC_H__
#define __XALLOC_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
void *xalloc(size_t size);
void *xalloc_try_alloc(size_t size);
void *xalloc0(size_t size);
void xfree(void *mem);
void *xrealloc(void *mem, size_t size);

void *xcalloc(size_t nitems, size_t size);


// #define xcalloc(num, item_size) calloc(num, item_size)

#ifdef __cplusplus
}
#endif

#endif // __XALLOC_H__
