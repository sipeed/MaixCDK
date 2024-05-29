#ifndef __MAIX_AVC2FLV
#define __MAIX_AVC2FLV

#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"

int maix_avc2flv_init(int max_buff_size);
int maix_avc2flv_deinit();
int maix_avc2flv_prepare(uint8_t *data, int data_size);
int maix_avc2flv_iterate(void **nalu, int *size);
int maix_avc2flv(void *nalu, int nalu_size, uint32_t pts, uint32_t dts, uint8_t **flv, int *flv_size);

// need free data after used
int maix_flv_get_tail(uint8_t **data, int *size);
// need free data after used
int maix_flv_get_header(int audio, int video, uint8_t **data, int *size);
#ifdef __cplusplus
}
#endif

#endif // __MAIX_AVC2FLV