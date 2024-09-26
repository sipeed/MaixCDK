#include "gcsv.h"
#include "string.h"

int gcsv_init(gcsv_handle_t *handle, char *filename, gcsv_header_t *header)
{
    if (!header || !handle) return -1;

    memset(handle, 0, sizeof(gcsv_handle_t));
    handle->f = fopen(filename, "w");
    if (!handle->f)
        return -1;
    memcpy(&handle->header, header, sizeof(gcsv_header_t));

    char header_str[512];
    snprintf(header_str, sizeof(header_str),
    "GYROFLOW IMU LOG\n"
    "version:%s\n"
    "id,%s\n"
    "orientation,%s\n"
    "tscale,%f\n"
    "gscale,%.11lf\n"
    "ascale,%.11lf\n"
    "t,gx,gy,gz,ax,ay,az\n",
    header->version, header->id, header->orientation,
    header->tscale, header->gscale, header->ascale);

    fwrite(header_str, strlen(header_str), 1, handle->f);
    return 0;
}

int gcsv_write(gcsv_handle_t *handle, gcsv_info_t *info)
{
    if (!info || !handle) return -1;

    char info_str[512];
    snprintf(info_str, sizeof(info_str),
    "%ld,%d,%d,%d,%d,%d,%d\n",
    info->t, info->gyro.x, info->gyro.y, info->gyro.z, info->acc.x, info->acc.y, info->acc.z);
    fwrite(info_str, (size_t)strlen(info_str), 1, handle->f);
    return 0;
}

int gcsv_deinit(gcsv_handle_t *handle)
{
    if (!handle) return -1;
    if (handle->f) {
        fclose(handle->f);
    }

    return 0;
}