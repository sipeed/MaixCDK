#ifndef __GCSV_H__
#define __GCSV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"

typedef struct {
    char version[16];
    char id[256];
    char orientation[8];
    double tscale;
    double gscale;
    double ascale;
    double mscale;
} gcsv_header_t;

typedef struct {
    uint64_t t;
    struct {
        int x;
        int y;
        int z;
    } gyro;
    struct {
        int x;
        int y;
        int z;
    } acc;
    struct {
        int x;
        int y;
        int z;
    } mag;
} gcsv_info_t;

typedef struct {
    FILE *f;
    gcsv_header_t header;
} gcsv_handle_t;

int gcsv_init(gcsv_handle_t *handle, char *filename, gcsv_header_t *header);
int gcsv_deinit(gcsv_handle_t *handle);
int gcsv_write(gcsv_handle_t *handle, gcsv_info_t *info);

#ifdef __cplusplus
}
#endif

#endif