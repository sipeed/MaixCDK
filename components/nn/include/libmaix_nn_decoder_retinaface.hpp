/*
    retinaface decoder
    @author neucrack@sipeed
    @date 2021-5-15 create for libmaix by neucrack
          2021-8-18 update for libmaix by neucrack
          2024-5-15 copy and edit for MaixCDK by neucrack
    @license MIT
*/

#ifndef __DECODER_RETINAFACE_H
#define __DECODER_RETINAFACE_H

#include "maix_nn_object.hpp"
#include <stdint.h>
#include <stdbool.h>

using namespace maix;

#define ANCHOR_SIZE_NUM 3
#define MIN_SIZE_LEN 6

typedef struct
{
    float variance[2];
    int steps[ANCHOR_SIZE_NUM];
    int min_sizes[ANCHOR_SIZE_NUM * 2];

    float nms;
    float score_thresh;
    int   input_w;
    int   input_h;

    // set by init func
    int   channel_num;
}libmaix_nn_decoder_retinaface_config_t;


/************ direct API ***********/
extern nn::ObjectFloat* retinaface_get_priorboxes(libmaix_nn_decoder_retinaface_config_t* config, int* boxes_num);
extern int retinaface_decode(float* net_out_loc, float* net_out_conf, float* net_out_landmark, nn::ObjectFloat* prior_boxes, std::vector<nn::Object> *faces, int* boxes_num, bool chw, libmaix_nn_decoder_retinaface_config_t* config);
extern int retinaface_get_channel_num(libmaix_nn_decoder_retinaface_config_t* config);


#endif

