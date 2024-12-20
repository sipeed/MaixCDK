

#include <math.h>
#include "libmaix_nn_decoder_retinaface.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define debug_line  //printf("%s:%d %s %s %s \r\n", __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__)

// int *steps = NULL; // config->steps;
// int *min_sizes = NULL; // config->min_sizes;

int min_size_len = MIN_SIZE_LEN;
int anchor_size_len = ANCHOR_SIZE_NUM;

static float overlap(float x1, float w1, float x2, float w2)
{
    float l1 = x1 - w1 / 2;
    float l2 = x2 - w2 / 2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = x1 + w1 / 2;
    float r2 = x2 + w2 / 2;
    float right = r1 < r2 ? r1 : r2;

    return right - left;
}

static float box_intersection(nn::Object* a, nn::Object* b)
{
    float w = overlap(a->x, a->w, b->x, b->w);
    float h = overlap(a->y, a->h, b->y, b->h);

    if (w < 0 || h < 0)
        return 0;
    return w * h;
}

static float box_union(nn::Object* a, nn::Object* b)
{
    float i = box_intersection(a, b);
    float u = a->w * a->h + b->w * b->h - i;

    return u;
}

static float box_iou(nn::Object* a, nn::Object* b)
{
    return box_intersection(a, b) / box_union(a, b);
}

typedef struct
{
    int index;
    int class_id;
    std::vector<nn::Object>* faces;
}sortable_box_t;


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

static int nms_comparator(const void *pa, const void *pb)
{
    sortable_box_t* a = (sortable_box_t *)pa;
    sortable_box_t* b = (sortable_box_t *)pb;
    float diff = a->faces->at(a->index).score - b->faces->at(b->index).score;

    // if (diff < 0)
    //     return 1;
    // else if (diff > 0)
    //     return -1;
    // return 0;
    return (int)(-*(int32_t*)(&diff));
}

#pragma GCC diagnostic pop

static void do_nms_sort(uint32_t boxes_number, float nms_value, float score_thresh, std::vector<nn::Object>* faces)
{
    uint32_t i = 0, j = 0, k = 0;
    sortable_box_t s[boxes_number];

    for (i = 0; i < boxes_number; ++i)
    {
        s[i].index = i;
        s[i].class_id = 0;
        s[i].faces = faces;
    }
    // for (k = 0; k < classes; ++k) // only one(face) class
    {
        for (i = 0; i < boxes_number; ++i)
            s[i].class_id = k;
        qsort(s, boxes_number, sizeof(sortable_box_t), nms_comparator);
        for (i = 0; i < boxes_number; ++i)
        {
            if (faces->at(s[i].index).score < score_thresh)
                continue;
            nn::Object* a = &faces->at(s[i].index);

            for (j = i + 1; j < boxes_number; ++j)
            {
                nn::Object* b = &faces->at(s[j].index);

                if (box_iou(a, b) > nms_value)
                    faces->at(s[j].index).score = 0;
            }
        }
    }
}

int retinaface_get_channel_num(libmaix_nn_decoder_retinaface_config_t* config)
{

    int anchors_size[anchor_size_len * 2];
    int anchor_num = 0;


    if(anchor_size_len * 2 != min_size_len)
    {
        int step_of_min_sizes [] = {3,2,2,3};
        for(int i = 0 ; i<anchor_size_len; i++)
        {
            anchors_size[i * 2] = ceil(config->input_h * 1.0 / config->steps[i]);
            anchors_size[i * 2 + 1] = ceil(config->input_w * 1.0 / config->steps[i]);
            anchor_num += anchors_size[i * 2] * anchors_size[i * 2 + 1] * step_of_min_sizes[i];
        }
    }

    else{
        // int step_of_min_sizes [] = {2,2,2,2};
        for(int i=0; i < anchor_size_len; ++i)
        {
            anchor_num += config->input_w / config->steps[i] * (config->input_h / config->steps[i]) * 2;
        }
    }
    debug_line;
    return anchor_num;
}

nn::ObjectFloat* retinaface_get_priorboxes(libmaix_nn_decoder_retinaface_config_t* config, int* boxes_num)
{

    int anchors_size[anchor_size_len * 2];
    int anchor_num = 0;
    int count = 0;

    if(anchor_size_len * 2 != min_size_len)
    {
        int step_of_min_sizes [] = {3,2,2,3};
        for(int i = 0 ; i<anchor_size_len; i++)
        {
            anchors_size[i * 2] = ceil(config->input_h * 1.0 / config->steps[i]);
            anchors_size[i * 2 + 1] = ceil(config->input_w * 1.0 / config->steps[i]);
            anchor_num += anchors_size[i * 2] * anchors_size[i * 2 + 1] * step_of_min_sizes[i];
        }
    }

    else{
        for(int i=0; i < anchor_size_len; ++i)
        {
            anchors_size[i * 2] = ceil(config->input_h * 1.0 / config->steps[i]);
            anchors_size[i * 2 + 1] = ceil(config->input_w * 1.0 / config->steps[i]);
            anchor_num += anchors_size[i * 2] * anchors_size[i * 2 + 1] * 2;
        }
    }
    *boxes_num = anchor_num;


    nn::ObjectFloat* boxes = (nn::ObjectFloat*)malloc(sizeof(nn::ObjectFloat) * anchor_num);
    if(!boxes)
    {
        printf("malloc fail\n");
        return NULL;
    }

    if(anchor_size_len *2 != min_size_len)
    {
        int start  = 0;
        int step_of_min_sizes [] = {3,2,2,3};

        for (int i=0 ; i < anchor_size_len;i++ )
        {
            for (int j=0 ; j < anchors_size[i*2];j++)
            {
                for(int k=0 ; k< anchors_size[i*2+1];k++)
                {
                    int end = start + step_of_min_sizes[i];
                    for(int l = start; l < end ; l++)
                    {
                        int min_size = config->min_sizes[l];
                        boxes[count].x = (k + 0.5) * config->steps[i] / config->input_w;
                        boxes[count].y = (j + 0.5) * config->steps[i] / config->input_h;
                        boxes[count].w = min_size * 1.0 / config->input_w;
                        boxes[count].h = min_size * 1.0 / config->input_h;
                        count++;
                    }

                }
            }
            start += step_of_min_sizes[i];
        }
        debug_line;

    }
    else
    {
        for(int i=0; i < anchor_size_len; ++i)
        {
            for(int j=0; j < anchors_size[i * 2]; ++j)
            {
                for(int k=0; k < anchors_size[i * 2 + 1]; ++k)
                {
                    for(int m=0; m < 2; ++m)
                    {
                        int min_size = config->min_sizes[i * 2 + m];
                        boxes[count].x = (k + 0.5) * config->steps[i] / config->input_w;
                        boxes[count].y = (j + 0.5) * config->steps[i] / config->input_h;
                        boxes[count].w = min_size * 1.0 / config->input_w;
                        boxes[count].h = min_size * 1.0 / config->input_h;
                        ++count;
                    }
                }
            }
        }

    }
    debug_line;

    return boxes;
}

// static void softmax(float *data, int stride, int n )
// {
//     int i;
//     // int diff;
//     // float e;
//     float sum = 0;
//     float largest_i = data[0];

//     for (i = 0; i < n; ++i)
//     {
//         if (data[i + stride] > largest_i)
//             largest_i = data[i + stride];
//     }
//     for (i = 0; i < n; ++i)
//     {
//         float value = expf(data[i + stride] - largest_i);
//         sum += value;
//         data[i + stride] = value;
//     }
//     for (i = 0; i < n; ++i)
// 	{
//         data[i + stride] /= sum;
// 	}
// }

int retinaface_decode(float* net_out_loc, float* net_out_conf, float* net_out_landmark, nn::ObjectFloat* prior_boxes, std::vector<nn::Object> *faces, int* boxes_num, bool chw, libmaix_nn_decoder_retinaface_config_t* config)
{
    int valid_boxes_count = 0;
    int all_boxes_num = *boxes_num;
    int idx = 0;
    debug_line;
    if(!chw) // hwc: [[[x, x,x,x,....], [y,y,y,y..][w....], [h...]]]
    {
        debug_line;
        /* 1 remove boxes which score < threshhold */
        for(int i=0; i < *boxes_num; ++i)
        {
            /* 1.1 softmax */
            // softmax(net_out_conf + i, all_boxes_num, 2);

            /* 1.2. decode conf score */
            faces->at(i).score = net_out_conf[all_boxes_num + i];

            /* 1.3 tag only copy valid faces info*/
            if(faces->at(i).score > config->score_thresh)
            {
                faces->at(valid_boxes_count).score = faces->at(i).score;
                faces->at(valid_boxes_count).class_id = i;
                ++valid_boxes_count;
            }
        }
        *boxes_num = valid_boxes_count;

        for(int i=0; i < *boxes_num; ++i)
        {
            idx = faces->at(i).class_id;

            /* 2. decode boxes*/
            faces->at(i).x = config->input_w * (prior_boxes[idx].x + net_out_loc[idx] * config->variance[0] * prior_boxes[idx].w);
            faces->at(i).y = config->input_h * (prior_boxes[idx].y + net_out_loc[idx + all_boxes_num] * config->variance[0] * prior_boxes[idx].h);
            faces->at(i).w = config->input_w * (prior_boxes[idx].w * exp(net_out_loc[idx + all_boxes_num * 2] * config->variance[1]));
            faces->at(i).h = config->input_h * (prior_boxes[idx].h * exp(net_out_loc[idx + all_boxes_num * 3] * config->variance[1]));
            faces->at(i).x = faces->at(i).x - faces->at(i).w / 2.0;
            faces->at(i).y = faces->at(i).y - faces->at(i).h / 2.0;

            /* 3. decode landmarks*/
            faces->at(i).points.push_back(config->input_w * (prior_boxes[idx].x + net_out_landmark[idx] * config->variance[0] * prior_boxes[idx].w));
            faces->at(i).points.push_back(config->input_h * (prior_boxes[idx].y + net_out_landmark[idx + all_boxes_num] * config->variance[0] * prior_boxes[idx].h));
            faces->at(i).points.push_back(config->input_w * (prior_boxes[idx].x + net_out_landmark[idx + all_boxes_num * 2] * config->variance[0] * prior_boxes[idx].w));
            faces->at(i).points.push_back(config->input_h * (prior_boxes[idx].y + net_out_landmark[idx + all_boxes_num * 3] * config->variance[0] * prior_boxes[idx].h));
            faces->at(i).points.push_back(config->input_w * (prior_boxes[idx].x + net_out_landmark[idx + all_boxes_num * 4] * config->variance[0] * prior_boxes[idx].w));
            faces->at(i).points.push_back(config->input_h * (prior_boxes[idx].y + net_out_landmark[idx + all_boxes_num * 5] * config->variance[0] * prior_boxes[idx].h));
            faces->at(i).points.push_back(config->input_w * (prior_boxes[idx].x + net_out_landmark[idx + all_boxes_num * 6] * config->variance[0] * prior_boxes[idx].w));
            faces->at(i).points.push_back(config->input_h * (prior_boxes[idx].y + net_out_landmark[idx + all_boxes_num * 7] * config->variance[0] * prior_boxes[idx].h));
            faces->at(i).points.push_back(config->input_w * (prior_boxes[idx].x + net_out_landmark[idx + all_boxes_num * 8] * config->variance[0] * prior_boxes[idx].w));
            faces->at(i).points.push_back(config->input_h * (prior_boxes[idx].y + net_out_landmark[idx + all_boxes_num * 9] * config->variance[0] * prior_boxes[idx].h));
            faces->at(i).class_id = 0;
        }
    }
    else    // chw: x,y,w,h......x,y,w,h
    {
        debug_line;
        /* 1 remove boxes which score < threshhold */
        // CALC_TIME_START();
        for(int i=0; i < *boxes_num; i++)
        {
            /* 1.1 softmax */
            // debug_line("%f, %f ==> ", net_out_conf[i * 2 ], net_out_conf[i * 2 + 1]);
            // softmax(net_out_conf + i * 2, 0, 2);
            // debug_line("%f, %f\n", net_out_conf[i * 2 ], net_out_conf[i * 2 + 1]);
            /* 1.2. decode conf score */
            faces->at(i).score = net_out_conf[i * 2 +1 ];

            /* 1.3 tag only copy valid faces info*/
            if(faces->at(i).score > config->score_thresh)
            {
                faces->at(valid_boxes_count).score = faces->at(i).score;
                faces->at(valid_boxes_count).class_id = i;
                ++valid_boxes_count;
            }
        }
        *boxes_num = valid_boxes_count;
        //  debug_line("[libmaix_nn decoder ] valid_boxes_count is %d\n",valid_boxes_count);
        // CALC_TIME_END("find valid boxes");
        // CALC_TIME_START();

        for(int i=0; i < *boxes_num; ++i)
        {
            idx = faces->at(i).class_id;

            /* 2. decode boxes*/
            faces->at(i).x = config->input_w * (prior_boxes[idx].x + net_out_loc[idx * 4] * config->variance[0] * prior_boxes[idx].w);
            faces->at(i).y = config->input_h * (prior_boxes[idx].y + net_out_loc[idx * 4 + 1] * config->variance[0] * prior_boxes[idx].h);
            faces->at(i).w = config->input_w * (prior_boxes[idx].w * exp(net_out_loc[idx * 4 + 2] * config->variance[1]));
            faces->at(i).h = config->input_h * (prior_boxes[idx].h * exp(net_out_loc[idx * 4 + 3] * config->variance[1]));
            faces->at(i).x = faces->at(i).x - faces->at(i).w / 2.0;
            faces->at(i).y = faces->at(i).y - faces->at(i).h / 2.0;
            // debug_line("%f %f %f %f, %f %f, %f %f\n", faces->at(i).box.x, faces->at(i).box.y, faces->at(i).box.w, faces->at(i).box.h, prior_boxes[i].w , prior_boxes[i].h, net_out_loc[i * 4 + 2], net_out_loc[i * 4 + 3]);

            /* 3. decode landmarks*/
            faces->at(i).points.push_back(config->input_w * (prior_boxes[idx].x + net_out_landmark[idx * 10] * config->variance[0] * prior_boxes[idx].w));
            faces->at(i).points.push_back(config->input_h * (prior_boxes[idx].y + net_out_landmark[idx * 10 + 1] * config->variance[0] * prior_boxes[idx].h));
            faces->at(i).points.push_back(config->input_w * (prior_boxes[idx].x + net_out_landmark[idx * 10 + 2] * config->variance[0] * prior_boxes[idx].w));
            faces->at(i).points.push_back(config->input_h * (prior_boxes[idx].y + net_out_landmark[idx * 10 + 3] * config->variance[0] * prior_boxes[idx].h));
            faces->at(i).points.push_back(config->input_w * (prior_boxes[idx].x + net_out_landmark[idx * 10 + 4] * config->variance[0] * prior_boxes[idx].w));
            faces->at(i).points.push_back(config->input_h * (prior_boxes[idx].y + net_out_landmark[idx * 10 + 5] * config->variance[0] * prior_boxes[idx].h));
            faces->at(i).points.push_back(config->input_w * (prior_boxes[idx].x + net_out_landmark[idx * 10 + 6] * config->variance[0] * prior_boxes[idx].w));
            faces->at(i).points.push_back(config->input_h * (prior_boxes[idx].y + net_out_landmark[idx * 10 + 7] * config->variance[0] * prior_boxes[idx].h));
            faces->at(i).points.push_back(config->input_w * (prior_boxes[idx].x + net_out_landmark[idx * 10 + 8] * config->variance[0] * prior_boxes[idx].w));
            faces->at(i).points.push_back(config->input_h * (prior_boxes[idx].y + net_out_landmark[idx * 10 + 9] * config->variance[0] * prior_boxes[idx].h));
            faces->at(i).class_id = 0;
        }
        // CALC_TIME_END("decode valid boxes");
    }
    /* 4. nms, remove boxes */
    // CALC_TIME_START();
    debug_line;
    do_nms_sort(*boxes_num, config->nms, config->score_thresh, faces);
    debug_line;
    // CALC_TIME_END("do nms");

    return 0;
}

