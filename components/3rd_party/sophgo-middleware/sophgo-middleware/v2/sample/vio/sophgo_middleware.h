#ifndef __SOPHGO_MIDDLEWARE_HPP__
#define __SOPHGO_MIDDLEWARE_HPP__

// init sys
int mmf_init(void);
int mmf_deinit(void);
bool mmf_is_init(void);

// manage vi channels(vi->vpssgroup->vpss->frame)
int mmf_get_vi_unused_channel(void);
int mmf_add_vi_channel(int ch, int width, int height, int format);
int mmf_del_vi_channel(int ch);
int mmf_del_vi_channel_all(void);
bool mmf_vi_chn_is_open(int ch);
int mmf_vi_aligned_width(int ch);

// get vi frame
int mmf_vi_frame_pop(int ch, void **data, int *len, int *width, int *height, int *format);
void mmf_vi_frame_free(int ch);

// manage vo channels
int mmf_get_vo_unused_channel(int layer);
int mmf_add_vo_channel(int layer, int ch, int width, int height, int format);
int mmf_del_vo_channel(int layer, int ch);
int mmf_del_vo_channel_all(int layer);
bool mmf_vo_channel_is_open(int layer, int ch);

// flush vo
int mmf_vo_frame_push(int layer, int ch, void *data, int len, int width, int height, int format);

// invert format
int mmf_invert_format_to_maix(int mmf_format);
int mmf_invert_format_to_mmf(int maix_format);

#endif // __SOPHGO_MIDDLEWARE_HPP__
