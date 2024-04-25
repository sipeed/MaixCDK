#ifndef __SOPHGO_MIDDLEWARE_HPP__
#define __SOPHGO_MIDDLEWARE_HPP__

typedef struct {
    uint8_t *data[8];
    int data_size[8];
    int count;
} mmf_h265_stream_t;

// init sys
int mmf_init(void);
int mmf_deinit(void);
bool mmf_is_init(void);

// manage vi channels(vi->vpssgroup->vpss->frame)
int mmf_get_vi_unused_channel(void);
int mmf_add_vi_channel(int ch, int width, int height, int format);
int mmf_del_vi_channel(int ch);
int mmf_del_vi_channel_all(void);
int mmf_reset_vi_channel(int ch, int width, int height, int format);
bool mmf_vi_chn_is_open(int ch);
int mmf_vi_aligned_width(int ch);
void mmf_set_vi_hmirror(int ch, bool en);
void mmf_set_vi_vflip(int ch, bool en);

// get vi frame
int mmf_vi_frame_pop(int ch, void **data, int *len, int *width, int *height, int *format);
void mmf_vi_frame_free(int ch);

// manage vo channels
int mmf_get_vo_unused_channel(int layer);
int mmf_add_vo_channel(int layer, int ch, int width, int height, int format);
int mmf_del_vo_channel(int layer, int ch);
int mmf_del_vo_channel_all(int layer);
bool mmf_vo_channel_is_open(int layer, int ch);
void mmf_set_vo_video_hmirror(int ch, bool en);
void mmf_set_vo_video_flip(int ch, bool en);

// flush vo
int mmf_vo_frame_push(int layer, int ch, void *data, int len, int width, int height, int format);

// rgn
bool mmf_rgn_is_init(void);
int mmf_region_init(int w, int h, int format);
int mmf_region_deinit(void);
int mmf_region_frame_push(int ch, void *data, int len);

// enc jpg
bool mmf_enc_jpg_is_init(void);
int mmf_enc_jpg_init(int ch, int w, int h, int format, int quality);
int mmf_enc_jpg_deinit(int ch);
int mmf_enc_jpg_push(int ch, uint8_t *data, int w, int h, int format);
int mmf_enc_jpg_pop(int ch, uint8_t **data, int *size);
int mmf_enc_jpg_free(int ch);

// enc h265
int mmf_enc_h265_init(int ch, int w, int h);
int mmf_enc_h265_deinit(int ch);
int mmf_enc_h265_push(int ch, uint8_t *data, int w, int h, int format);
int mmf_enc_h265_pop(int ch, mmf_h265_stream_t *stream);
int mmf_enc_h265_free(int ch);

// invert format
int mmf_invert_format_to_maix(int mmf_format);
int mmf_invert_format_to_mmf(int maix_format);

// config vb
int mmf_vb_config_of_vi(uint32_t size, uint32_t count);     // must be run before mmf_init()
int mmf_vb_config_of_vo(uint32_t size, uint32_t count);     // must be run before mmf_init()
int mmf_vb_config_of_private(uint32_t size, uint32_t count);    // must be run before mmf_init()

// config isp
int mmf_set_exp_mode(int ch, int mode);
int mmf_get_exp_mode(int ch);
int mmf_get_exptime(int ch, uint32_t *exptime);
int mmf_set_exptime(int ch, uint32_t exptime);
int mmf_get_iso_num(int ch, uint32_t *iso_num);
int mmf_set_iso_num(int ch, uint32_t iso_num);
int mmf_get_exptime_and_iso(int ch, uint32_t *exptime, uint32_t *iso_num);
int mmf_set_exptime_and_iso(int ch, uint32_t exptime, uint32_t iso_num);
void mmf_set_constrast(int ch, uint32_t val);
void mmf_set_saturation(int ch, uint32_t val);
void mmf_set_luma(int ch, uint32_t val);

// sensor info
int mmf_get_sensor_id(void);

#endif // __SOPHGO_MIDDLEWARE_HPP__
