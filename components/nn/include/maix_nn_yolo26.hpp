/**
 * @author Tao@sipeed, modified for YOLO26
 * @copyright Sipeed Ltd 2026-
 * @license Apache 2.0
 * @update 2026: YOLO26 with platform-specific SIMD optimization.
 *                MaixCAM2: NEON, MaixCAM: RVV, Others: Serial
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"
#include <algorithm>

#if PLATFORM_MAIXCAM2
    #include <arm_neon.h>
    #define USE_NEON_OPTIMIZATION 1
    #define USE_RVV_OPTIMIZATION 0
#elif PLATFORM_MAIXCAM
    #include <riscv_vector.h>
    #define USE_NEON_OPTIMIZATION 0
    #define USE_RVV_OPTIMIZATION 1
#else
    #define USE_NEON_OPTIMIZATION 0
    #define USE_RVV_OPTIMIZATION 0
#endif

namespace maix::nn
{
    /**
     * YOLO26 class for object detection
     * @maixpy maix.nn.YOLO26
     */
    class YOLO26
    {
    public:
        /**
         * Constructor of YOLO26 class
         * @maixpy maix.nn.YOLO26.__init__
         * @maixcdk maix.nn.YOLO26.YOLO26
         */
        YOLO26(const string &model = "", bool dual_buff = true)
        {
            _model = nullptr;
            _dual_buff = dual_buff;
            if (!model.empty())
            {
                err::Err e = load(model);
                err::check_raise(e, "load model failed");
            }
            
            // Log optimization method
#if USE_NEON_OPTIMIZATION
            log::info("YOLO26 using NEON optimization (MaixCAM2)");
#elif USE_RVV_OPTIMIZATION
            log::info("YOLO26 using RVV optimization (MaixCAM)");
#else
            log::info("YOLO26 using serial processing");
#endif
        }

        /**
         * Destructor of YOLO26 class
         * @maixcdk maix.nn.YOLO26.~YOLO26
         */
        ~YOLO26()
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
        }

        /**
         * Load model from file
         * @return err::Err
         * @maixpy maix.nn.YOLO26.load
         */
        err::Err load(const string &model)
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
            
            _model = new nn::NN(model, _dual_buff);
            err::check_null_raise(_model, "create model failed");
            
            _extra_info = _model->extra_info();
            
            // Check model type (lenient for compatibility)
            if (_extra_info.count("model_type"))
            {
                std::string model_type = _extra_info["model_type"];
                if (model_type.find("yolo26") == std::string::npos && 
                    model_type.find("YOLO26") == std::string::npos)
                {
                    log::warn("model_type is '%s', expected 'yolo26'. Trying anyway...", 
                              model_type.c_str());
                }
            }
            
            // Parse input type (with defaults)
            if (_extra_info.count("input_type"))
            {
                std::string input_type = _extra_info["input_type"];
                if (input_type == "rgb")
                    _input_img_fmt = maix::image::FMT_RGB888;
                else if (input_type == "bgr")
                    _input_img_fmt = maix::image::FMT_BGR888;
                else
                {
                    log::warn("Unknown input_type '%s', using RGB", input_type.c_str());
                    _input_img_fmt = maix::image::FMT_RGB888;
                }
            }
            else
            {
                _input_img_fmt = maix::image::FMT_RGB888;
            }
            
            // Parse mean and scale
            if (_extra_info.count("mean"))
                _parse_float_list(_extra_info["mean"], mean);
            else
                mean = {0.0f, 0.0f, 0.0f};
            
            if (_extra_info.count("scale"))
                _parse_float_list(_extra_info["scale"], scale);
            else
                scale = {1.0f/255.0f, 1.0f/255.0f, 1.0f/255.0f};
            
            // Parse labels
            err::Err e = _model->extra_info_labels(labels);
            if (e != err::Err::ERR_NONE || labels.size() == 0)
            {
                log::warn("labels not in metadata, will infer from output");
            }
            
            // Get input size
            std::vector<nn::LayerInfo> inputs = _model->inputs_info();
            _input_size = (inputs[0].shape[3] <= 4) ? 
                image::Size(inputs[0].shape[2], inputs[0].shape[1]) :
                image::Size(inputs[0].shape[3], inputs[0].shape[2]);
            
            // Parse output nodes
            e = _parse_output_nodes();
            err::check_raise(e, "parse output nodes failed");
            
            log::info("YOLO26 loaded: %dx%d, %d classes%s", 
                      _input_size.width(), _input_size.height(), labels.size(),
                      _is_nchw ? ", NCHW" : ", NHWC");
            
            return err::ERR_NONE;
        }

        /**
         * Detect objects from image
         * @param conf_th Confidence threshold, default 0.5.
         * @param iou_th IoU threshold (unused, kept for API compatibility).
         * @param sort Sort result (unused, kept for API compatibility).
         * @return Object list. In C++, you should delete it after use.
         * @maixpy maix.nn.YOLO26.detect
         */
        std::vector<nn::Object> *detect(image::Image &img, float conf_th = 0.5, 
                                        float iou_th = 0.45, maix::image::Fit fit = maix::image::FIT_CONTAIN, 
                                        int sort = 0)
        {
            _conf_th = conf_th;
            
            err::check_bool_raise(img.format() == _input_img_fmt, 
                                  "image format not match");
            
            tensor::Tensors *outputs = _model->forward_image(img, mean, scale, fit, false);

            if (!outputs)
                return new std::vector<nn::Object>();
            
            std::vector<nn::Object> *res = _post_process(outputs, img.width(), img.height(), fit);
            delete outputs;
            
            return res;
        }

        /**
         * Get model input size
         * @maixpy maix.nn.YOLO26.input_size
         */
        image::Size input_size() { return _input_size; }

        /**
         * Get model input width
         * @maixpy maix.nn.YOLO26.input_width
         */
        int input_width() { return _input_size.width(); }

        /**
         * Get model input height
         * @maixpy maix.nn.YOLO26.input_height
         */
        int input_height() { return _input_size.height(); }

        /**
         * Get input image format
         * @maixpy maix.nn.YOLO26.input_format
         */
        image::Format input_format() { return _input_img_fmt; }

    public:
        /**
         * Labels list
         * @maixpy maix.nn.YOLO26.labels
         */
        std::vector<string> labels;

        /**
         * Label file path
         * @maixpy maix.nn.YOLO26.label_path
         */
        std::string label_path;

        /**
         * Get mean value, list type
         * @maixpy maix.nn.YOLO26.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.YOLO26.scale
         */
        std::vector<float> scale;

    private:
        image::Size _input_size;
        image::Format _input_img_fmt;
        nn::NN *_model;
        std::map<string, string> _extra_info;
        float _conf_th = 0.5;
        bool _dual_buff;
        bool _is_nchw = false;
        
        static constexpr float LOGIT_THRESHOLD = -0.2f;
        
        struct OutputNodes
        {
            std::string bbox[3];
            std::string cls[3];
            int grid_sizes[3][2];
        } _output_nodes;

        /**
         * Parse output nodes from model
         */
        err::Err _parse_output_nodes()
        {
            std::vector<nn::LayerInfo> outputs = _model->outputs_info();
            err::check_bool_raise(outputs.size() >= 6, "need at least 6 outputs");
            
            struct OutputInfo { std::string name; int h, w, c; };
            std::vector<OutputInfo> bbox_outputs, cls_outputs;
            
            // Classify outputs by channel count
            for (const auto &output : outputs)
            {
                if (output.shape.size() != 4) continue;
                
                OutputInfo info;
                info.name = output.name;
                
#if PLATFORM_MAIXCAM2
                // MaixCAM2: Always NHWC format
                info.h = output.shape[1];
                info.w = output.shape[2];
                info.c = output.shape[3];
#else
                // Other platforms: Auto-detect NCHW vs NHWC
                int dim1 = output.shape[1];
                int dim2 = output.shape[2];
                int dim3 = output.shape[3];
                
                // 改进的格式检测逻辑
                bool is_channel_first = false;
                
                // 检查 dim1 是否像是通道数
                if (dim1 == 4 || dim1 == 80 || 
                    (labels.size() > 0 && dim1 == (int)labels.size()))
                {
                    is_channel_first = true;
                }
                // 或者 dim1 明显小于 dim2 和 dim3
                else if (dim1 <= 100 && dim1 < dim2 && dim1 < dim3)
                {
                    is_channel_first = true;
                }
                
                if (is_channel_first)
                {
                    // NCHW: 1 x C x H x W
                    _is_nchw = true;
                    info.c = dim1;
                    info.h = dim2;
                    info.w = dim3;
                }
                else
                {
                    // NHWC: 1 x H x W x C
                    info.h = dim1;
                    info.w = dim2;
                    info.c = dim3;
                }
#endif
                
                if (info.c == 4)
                    bbox_outputs.push_back(info);
                else if (info.c == (int)labels.size() || info.c == 80)
                    cls_outputs.push_back(info);
            }
            
            err::check_bool_raise(bbox_outputs.size() == 3 && cls_outputs.size() == 3,
                                  "need 3 bbox and 3 cls outputs");
            
            // Sort by grid size (largest first)
            auto sort_fn = [](const OutputInfo &a, const OutputInfo &b) {
                return (a.h * a.w) > (b.h * b.w);
            };
            std::sort(bbox_outputs.begin(), bbox_outputs.end(), sort_fn);
            std::sort(cls_outputs.begin(), cls_outputs.end(), sort_fn);
            
            // Store node info
            for (int i = 0; i < 3; i++)
            {
                err::check_bool_raise(bbox_outputs[i].h == cls_outputs[i].h && 
                                      bbox_outputs[i].w == cls_outputs[i].w,
                                      "bbox and cls grid size mismatch");
                
                _output_nodes.bbox[i] = bbox_outputs[i].name;
                _output_nodes.cls[i] = cls_outputs[i].name;
                _output_nodes.grid_sizes[i][0] = bbox_outputs[i].w;
                _output_nodes.grid_sizes[i][1] = bbox_outputs[i].h;
            }
            
            // Infer labels if not set
            int num_classes = cls_outputs[0].c;
            if (labels.size() == 0)
            {
                for (int i = 0; i < num_classes; i++)
                {
                    labels.push_back("class_" + std::to_string(i));
                }
            }
            
            return err::ERR_NONE;
        }

        /**
         * Post process for YOLO26 output
         */
        std::vector<nn::Object> *_post_process(tensor::Tensors *outputs, int img_w, int img_h, 
                                                maix::image::Fit fit)
        {
            std::vector<nn::Object> *objects = new std::vector<nn::Object>();
            
            // Process each scale
            for (int i = 0; i < 3; i++)
            {
                float *bbox = (float *)(*outputs)[_output_nodes.bbox[i]].data();
                float *cls = (float *)(*outputs)[_output_nodes.cls[i]].data();
                int stride = _input_size.width() / _output_nodes.grid_sizes[i][0];
                int fw = _output_nodes.grid_sizes[i][0];
                int fh = _output_nodes.grid_sizes[i][1];
                
                _generate_proposals(stride, fw, fh, bbox, cls, labels.size(), *objects);
            }
            
            // Correct bbox to original image size
            if (objects->size() > 0)
                _correct_bbox(*objects, img_w, img_h, fit);
            
            return objects;
        }

        /**
         * Generate proposals with platform-optimized implementation
         */
        void _generate_proposals(int stride, int fw, int fh,
                                  const float *bbox, const float *cls,
                                  int num_class, std::vector<nn::Object> &objs)
        {
            const int total = fw * fh;
            const float stride_f = (float)stride;
            
#if USE_NEON_OPTIMIZATION
            // ========== NEON optimized version for MaixCAM2 (NHWC only) ==========
            for (int i = 0; i < total; i++)
            {
                const float *c = cls + i * num_class;
                
                // Prefetch next iteration data
                if (i + 4 < total)
                {
                    __builtin_prefetch(cls + (i + 4) * num_class, 0, 1);
                }
                
                // Find max logit using NEON
                float max_logit = _find_max_neon(c, num_class);
                
                // Early exit
                if (max_logit < LOGIT_THRESHOLD)
                    continue;
                
                // Find class id
                int class_id = 0;
                for (int j = 1; j < num_class; j++)
                {
                    if (c[j] > c[class_id])
                        class_id = j;
                }
                
                // Check confidence
                float score = _sigmoid(max_logit);
                if (score <= _conf_th)
                    continue;
                
                // Calculate bbox
                int ax = i % fw;
                int ay = i / fw;
                const float *b = bbox + i * 4;
                
                float cx = (ax + 0.5f) * stride_f;
                float cy = (ay + 0.5f) * stride_f;
                float x = cx - b[0] * stride_f;
                float y = cy - b[1] * stride_f;
                float w = (b[0] + b[2]) * stride_f;
                float h = (b[1] + b[3]) * stride_f;
                
                // Clamp to input size
                x = std::max(0.0f, x);
                y = std::max(0.0f, y);
                w = std::min(w, (float)_input_size.width() - x);
                h = std::min(h, (float)_input_size.height() - y);
                
                if (w > 0 && h > 0)
                    objs.push_back(Object(x, y, w, h, class_id, score));
            }
            
#elif USE_RVV_OPTIMIZATION
            // ========== RVV optimized version for MaixCAM (NCHW) ==========
            
            if (_is_nchw)
            {
                // NCHW format processing with RVV
                for (int i = 0; i < total; i++)
                {
                    int ax = i % fw;
                    int ay = i / fw;
                    
                    // Find max logit and class id using RVV
                    float max_logit;
                    int class_id;
                    _find_max_class_rvv_nchw(cls, num_class, fh, fw, ay, ax, max_logit, class_id);
                    
                    // Early exit
                    if (max_logit < LOGIT_THRESHOLD)
                        continue;
                    
                    // Check confidence
                    float score = _sigmoid(max_logit);
                    if (score <= _conf_th)
                        continue;
                    
                    // Get bbox
                    float b[4];
                    b[0] = bbox[0 * fh * fw + ay * fw + ax];
                    b[1] = bbox[1 * fh * fw + ay * fw + ax];
                    b[2] = bbox[2 * fh * fw + ay * fw + ax];
                    b[3] = bbox[3 * fh * fw + ay * fw + ax];
                    
                    // Calculate bbox
                    float cx = (ax + 0.5f) * stride_f;
                    float cy = (ay + 0.5f) * stride_f;
                    float x = cx - b[0] * stride_f;
                    float y = cy - b[1] * stride_f;
                    float w = (b[0] + b[2]) * stride_f;
                    float h = (b[1] + b[3]) * stride_f;
                    
                    // Clamp to input size
                    x = std::max(0.0f, x);
                    y = std::max(0.0f, y);
                    w = std::min(w, (float)_input_size.width() - x);
                    h = std::min(h, (float)_input_size.height() - y);
                    
                    if (w > 0 && h > 0)
                        objs.push_back(Object(x, y, w, h, class_id, score));
                }
            }
            else
            {
                // NHWC format processing with RVV
                for (int i = 0; i < total; i++)
                {
                    int ax = i % fw;
                    int ay = i / fw;
                    
                    const float *c = cls + i * num_class;
                    const float *b = bbox + i * 4;
                    
                    // Find max logit and class id using RVV
                    float max_logit;
                    int class_id;
                    _find_max_class_rvv_nhwc(c, num_class, max_logit, class_id);
                    
                    // Early exit
                    if (max_logit < LOGIT_THRESHOLD)
                        continue;
                    
                    // Check confidence
                    float score = _sigmoid(max_logit);
                    if (score <= _conf_th)
                        continue;
                    
                    // Calculate bbox
                    float cx = (ax + 0.5f) * stride_f;
                    float cy = (ay + 0.5f) * stride_f;
                    float x = cx - b[0] * stride_f;
                    float y = cy - b[1] * stride_f;
                    float w = (b[0] + b[2]) * stride_f;
                    float h = (b[1] + b[3]) * stride_f;
                    
                    // Clamp to input size
                    x = std::max(0.0f, x);
                    y = std::max(0.0f, y);
                    w = std::min(w, (float)_input_size.width() - x);
                    h = std::min(h, (float)_input_size.height() - y);
                    
                    if (w > 0 && h > 0)
                        objs.push_back(Object(x, y, w, h, class_id, score));
                }
            }
            
#else
            // ========== Serial version (fallback) ==========
            for (int i = 0; i < total; i++)
            {
                int ax = i % fw;
                int ay = i / fw;
                
                // Get class scores (handle NCHW vs NHWC)
                float class_scores[80];
                const float *c;
                
                if (_is_nchw)
                {
                    for (int j = 0; j < num_class; j++)
                    {
                        class_scores[j] = cls[j * fh * fw + ay * fw + ax];
                    }
                    c = class_scores;
                }
                else
                {
                    c = cls + i * num_class;
                }
                
                // Find max logit and class id
                float max_logit = c[0];
                int class_id = 0;
                for (int j = 1; j < num_class; j++)
                {
                    if (c[j] > max_logit)
                    {
                        max_logit = c[j];
                        class_id = j;
                    }
                }
                
                // Early exit
                if (max_logit < LOGIT_THRESHOLD)
                    continue;
                
                // Check confidence
                float score = _sigmoid(max_logit);
                if (score <= _conf_th)
                    continue;
                
                // Get bbox (handle NCHW vs NHWC)
                float b[4];
                if (_is_nchw)
                {
                    b[0] = bbox[0 * fh * fw + ay * fw + ax];
                    b[1] = bbox[1 * fh * fw + ay * fw + ax];
                    b[2] = bbox[2 * fh * fw + ay * fw + ax];
                    b[3] = bbox[3 * fh * fw + ay * fw + ax];
                }
                else
                {
                    const float *b_ptr = bbox + i * 4;
                    b[0] = b_ptr[0];
                    b[1] = b_ptr[1];
                    b[2] = b_ptr[2];
                    b[3] = b_ptr[3];
                }
                
                // Calculate bbox
                float cx = (ax + 0.5f) * stride_f;
                float cy = (ay + 0.5f) * stride_f;
                float x = cx - b[0] * stride_f;
                float y = cy - b[1] * stride_f;
                float w = (b[0] + b[2]) * stride_f;
                float h = (b[1] + b[3]) * stride_f;
                
                // Clamp to input size
                x = std::max(0.0f, x);
                y = std::max(0.0f, y);
                w = std::min(w, (float)_input_size.width() - x);
                h = std::min(h, (float)_input_size.height() - y);
                
                if (w > 0 && h > 0)
                    objs.push_back(Object(x, y, w, h, class_id, score));
            }
#endif
        }

#if USE_NEON_OPTIMIZATION
        /**
         * Find max value using NEON SIMD (MaixCAM2 only)
         */
        inline float _find_max_neon(const float *data, int count)
        {
            if (count <= 0) return -1e9f;
            
            int vec_count = count / 16 * 16;
            float32x4_t vmax0 = vdupq_n_f32(-1e9f);
            float32x4_t vmax1 = vdupq_n_f32(-1e9f);
            float32x4_t vmax2 = vdupq_n_f32(-1e9f);
            float32x4_t vmax3 = vdupq_n_f32(-1e9f);
            
            for (int j = 0; j < vec_count; j += 16)
            {
                vmax0 = vmaxq_f32(vmax0, vld1q_f32(data + j));
                vmax1 = vmaxq_f32(vmax1, vld1q_f32(data + j + 4));
                vmax2 = vmaxq_f32(vmax2, vld1q_f32(data + j + 8));
                vmax3 = vmaxq_f32(vmax3, vld1q_f32(data + j + 12));
            }
            
            vmax0 = vmaxq_f32(vmax0, vmax1);
            vmax2 = vmaxq_f32(vmax2, vmax3);
            vmax0 = vmaxq_f32(vmax0, vmax2);
            
            float32x2_t vmax_pair = vpmax_f32(vget_low_f32(vmax0), vget_high_f32(vmax0));
            vmax_pair = vpmax_f32(vmax_pair, vmax_pair);
            float max_val = vget_lane_f32(vmax_pair, 0);
            
            for (int j = vec_count; j < count; j++)
                max_val = std::max(max_val, data[j]);
            
            return max_val;
        }
#endif

#if USE_RVV_OPTIMIZATION
        /**
         * Find max value and class id using RVV SIMD for NHWC format (MaixCAM)
         */
        inline void _find_max_class_rvv_nhwc(const float *data, int count, 
                                              float &max_val, int &max_idx)
        {
            max_val = -1e9f;
            max_idx = 0;
            
            size_t vl;
            int i = 0;
            
            // Vector loop
            while (i < count)
            {
                vl = vsetvl_e32m8(count - i);
                vfloat32m8_t vec = vle32_v_f32m8(data + i, vl);
                
                // Find max in this vector segment
                vfloat32m1_t vmax = vfredmax_vs_f32m8_f32m1(vfmv_s_f_f32m1(vfmv_v_f_f32m1(-1e9f, 1), -1e9f, 1), vec, vfmv_v_f_f32m1(-1e9f, 1), vl);
                float local_max = vfmv_f_s_f32m1_f32(vmax);
                
                // Check if this segment has a new max
                if (local_max > max_val)
                {
                    max_val = local_max;
                    // Find the index within this segment
                    for (size_t j = 0; j < vl; j++)
                    {
                        if (data[i + j] == local_max)
                        {
                            max_idx = i + j;
                            break;
                        }
                    }
                }
                
                i += vl;
            }
        }
        
        /**
         * Find max value and class id using RVV SIMD for NCHW format (MaixCAM)
         */
        inline void _find_max_class_rvv_nchw(const float *cls, int num_class, 
                                              int fh, int fw, int ay, int ax,
                                              float &max_val, int &max_idx)
        {
            max_val = -1e9f;
            max_idx = 0;
            
            const int spatial_offset = ay * fw + ax;
            const int spatial_size = fh * fw;
            
            // Gather class scores from NCHW layout
            // For small num_class (80), scalar is often faster
            if (num_class <= 80)
            {
                for (int j = 0; j < num_class; j++)
                {
                    float val = cls[j * spatial_size + spatial_offset];
                    if (val > max_val)
                    {
                        max_val = val;
                        max_idx = j;
                    }
                }
            }
            else
            {
                // For large num_class, use RVV with strided load
                int i = 0;
                size_t vl;
                
                while (i < num_class)
                {
                    vl = vsetvl_e32m8(num_class - i);
                    
                    // Load with stride (gather from different channels)
                    vfloat32m8_t vec = vlse32_v_f32m8(cls + i * spatial_size + spatial_offset, 
                                                       spatial_size * sizeof(float), vl);
                    
                    // Find max in this vector segment
                    vfloat32m1_t vmax = vfredmax_vs_f32m8_f32m1(vfmv_s_f_f32m1(vfmv_v_f_f32m1(-1e9f, 1), -1e9f, 1), vec, vfmv_v_f_f32m1(-1e9f, 1), vl);
                    float local_max = vfmv_f_s_f32m1_f32(vmax);
                    
                    if (local_max > max_val)
                    {
                        max_val = local_max;
                        // Find exact index
                        for (size_t j = 0; j < vl; j++)
                        {
                            float val = cls[(i + j) * spatial_size + spatial_offset];
                            if (val == local_max)
                            {
                                max_idx = i + j;
                                break;
                            }
                        }
                    }
                    
                    i += vl;
                }
            }
        }
#endif

        /**
         * Correct bbox to original image size
         */
        void _correct_bbox(std::vector<nn::Object> &objs, int img_w, int img_h, maix::image::Fit fit)
        {
            if (img_w == _input_size.width() && img_h == _input_size.height())
                return;
            
            float scale_x = (float)_input_size.width() / img_w;
            float scale_y = (float)_input_size.height() / img_h;
            
            if (fit == maix::image::FIT_FILL)
            {
                for (size_t i = 0; i < objs.size(); i++)
                {
                    auto &obj = objs[i];
                    obj.x /= scale_x;
                    obj.y /= scale_y;
                    obj.w /= scale_x;
                    obj.h /= scale_y;
                    _clamp_bbox(obj, img_w, img_h);
                }
            }
            else if (fit == maix::image::FIT_CONTAIN)
            {
                float scale = std::min(scale_x, scale_y);
                float pad_w = (_input_size.width() - img_w * scale) / 2.0f;
                float pad_h = (_input_size.height() - img_h * scale) / 2.0f;
                
                for (size_t i = 0; i < objs.size(); i++)
                {
                    auto &obj = objs[i];
                    obj.x = (obj.x - pad_w) / scale;
                    obj.y = (obj.y - pad_h) / scale;
                    obj.w /= scale;
                    obj.h /= scale;
                    _clamp_bbox(obj, img_w, img_h);
                }
            }
            else if (fit == maix::image::FIT_COVER)
            {
                float scale = std::max(scale_x, scale_y);
                float pad_w = (img_w * scale - _input_size.width()) / 2.0f;
                float pad_h = (img_h * scale - _input_size.height()) / 2.0f;

                for (size_t i = 0; i < objs.size(); i++)
                {
                    auto &obj = objs[i];
                    obj.x = (obj.x + pad_w) / scale;
                    obj.y = (obj.y + pad_h) / scale;
                    obj.w /= scale;
                    obj.h /= scale;
                    _clamp_bbox(obj, img_w, img_h);
                }
            }
        }

        /**
         * Clamp bbox to image boundaries
         */
        inline void _clamp_bbox(nn::Object &obj, int img_w, int img_h)
        {
            if (obj.x < 0) { obj.w += obj.x; obj.x = 0; }
            if (obj.y < 0) { obj.h += obj.y; obj.y = 0; }
            if (obj.x + obj.w > img_w) obj.w = img_w - obj.x;
            if (obj.y + obj.h > img_h) obj.h = img_h - obj.y;
        }

        /**
         * Sigmoid function
         */
        inline static float _sigmoid(float x)
        {
            return 1.0f / (1.0f + expf(-x));
        }

        /**
         * Parse float list from string
         */
        void _parse_float_list(const std::string &str, std::vector<float> &vec)
        {
            size_t start = 0, end;
            while ((end = str.find(',', start)) != std::string::npos)
            {
                vec.push_back(std::stof(str.substr(start, end - start)));
                start = end + 1;
            }
            vec.push_back(std::stof(str.substr(start)));
        }
    };

} // namespace maix::nn