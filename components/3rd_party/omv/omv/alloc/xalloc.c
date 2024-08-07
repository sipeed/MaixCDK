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
#include <string.h>
// #include "py/runtime.h"
// #include "py/gc.h"
// #include "py/mphal.h"
#include "xalloc.h"
#include <stdio.h>
#include <stdlib.h>
// #include "imlib_io.h"

static void xalloc_fail(size_t size)
{
    printf("MemoryError :memory allocation failed, allocating %ld bytes", size);
    // mp_raise_msg_varg(&mp_type_MemoryError,
    //         MP_ERROR_TEXT("memory allocation failed, allocating %u bytes"), (uint)size);
}

// returns null pointer without error if size==0
void *xalloc(size_t size)
{
    void *mem = malloc(size);
    if (size && (mem == NULL)) {
        xalloc_fail(size);
    }
    return mem;
}

// returns null pointer without error if size==0
void *xalloc_try_alloc(size_t size)
{
    return malloc(size);
}

// returns null pointer without error if size==0
void *xalloc0(size_t size)
{
    void *mem = malloc(size);
    if (size && (mem == NULL)) {
        xalloc_fail(size);
    }
    memset(mem, 0, size);
    return mem;
}

// returns without error if mem==null
void xfree(void *mem)
{
    free(mem);
}

// returns null pointer without error if size==0
// allocs if mem==null and size!=0
// frees if mem!=null and size==0
void *xrealloc(void *mem, size_t size)
{
    mem = realloc(mem, size);
    if (size && (mem == NULL)) {
        xalloc_fail(size);
    }
    return mem;
}
void *xcalloc(size_t nitems, size_t size)
{
    void *mem = calloc(nitems, size);
    if (size && (mem == NULL)) {
        xalloc_fail(size);
    }
    return mem;
}

