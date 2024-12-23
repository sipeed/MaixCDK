/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2024.5.17: Create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"
#include <tuple>
#include "maix_nn_face_detector.hpp"
#include "maix_nn_retinaface.hpp"
#include "maix_nn_yolov8.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

static std::string _get_detect_model(const std::string &path)
{
    std::string model_type = "";

    // 判断 path 是否是 .mud 格式
    if (path.substr(path.find_last_of(".") + 1) != "mud") {
        return model_type;
    }

    // 打开文件
    std::ifstream file(path);
    if (!file.is_open()) {
        return model_type;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 查找以 "model_type" 开头的行
        if (line.find("model_type") == 0) {
            // 查找等号的位置
            size_t pos = line.find("=");
            if (pos != std::string::npos) {
                // 获取等号后面的部分
                model_type = line.substr(pos + 1);

                // 去掉前后的空白字符（空格、回车等）
                model_type.erase(std::remove_if(model_type.begin(), model_type.end(), ::isspace), model_type.end());

                break;  // 找到之后直接退出
            }
        }
    }

    file.close();
    return model_type;
}

namespace maix::nn
{
    /**
     * Face object
     * @maixpy maix.nn.FaceObject
     */
    class FaceObject
    {
    private:
        /* data */
    public:
        /**
         * Constructor
         * @maixpy maix.nn.FaceObject.__init__
         * @maixcdk maix.nn.FaceObject.FaceObject
         */
        FaceObject(int x = 0, int y = 0, int w = 0, int h = 0, int class_id = 0, float score = 0, std::vector<int> points = std::vector<int>(), std::vector<float> feature = std::vector<float>(), image::Image face = image::Image())
            : x(x), y(y), w(w), h(h), class_id(class_id), score(score), points(points), feature(feature), face(face)
        {
        }

        ~FaceObject()
        {
        }

        /**
         * FaceObject info to string
         * @return FaceObject info string
         * @maixpy maix.nn.FaceObject.__str__
         * @maixcdk maix.nn.FaceObject.to_str
         */
        std::string to_str()
        {
            return "x: " + std::to_string(x) + ", y: " + std::to_string(y) + ", w: " + std::to_string(w) + ", h: " + std::to_string(h) + ", class_id: " + std::to_string(class_id) + ", score: " + std::to_string(score);
        }

        /**
         * FaceObject left top coordinate x
         * @maixpy maix.nn.FaceObject.x
         */
        int x;

        /**
         * FaceObject left top coordinate y
         * @maixpy maix.nn.FaceObject.y
         */
        int y;

        /**
         * FaceObject width
         * @maixpy maix.nn.FaceObject.w
         */
        int w;

        /**
         * FaceObject height
         * @maixpy maix.nn.FaceObject.h
         */
        int h;

        /**
         * FaceObject class id
         * @maixpy maix.nn.FaceObject.class_id
         */
        int class_id;

        /**
         * FaceObject score
         * @maixpy maix.nn.FaceObject.score
         */
        float score;

        /**
         * keypoints
         * @maixpy maix.nn.FaceObject.points
         */
        std::vector<int> points;

        /**
         * feature, float list type
         * @maixpy maix.nn.FaceObject.feature
         */
        std::vector<float> feature;

        /**
         * face image
         * @maixpy maix.nn.FaceObject.face
        */
        image::Image face;

    private:
    };

    /**
     * Objects Class for detect result
     * @maixpy maix.nn.FaceObjects
     */
    class FaceObjects
    {
    public:
        /**
         * Constructor of FaceObjects class
         * @maixpy maix.nn.FaceObjects.__init__
         * @maixcdk maix.nn.FaceObjects.FaceObjects
         */
        FaceObjects()
        {
        }

        ~FaceObjects()
        {
            for (FaceObject *obj : objs)
            {
                delete obj;
            }
        }

        /**
         * Add object to FaceObjects
         * @throw Throw exception if no memory
         * @maixpy maix.nn.FaceObjects.add
         */
        nn::FaceObject &add(int x = 0, int y = 0, int w = 0, int h = 0, int class_id = 0, float score = 0, std::vector<int> points = std::vector<int>(), std::vector<float> feature = std::vector<float>(), image::Image face = image::Image())
        {
            FaceObject *obj = new FaceObject(x, y, w, h, class_id, score, points, feature, face);
            if(!obj)
                throw err::Exception(err::ERR_NO_MEM);
            objs.push_back(obj);
            return *obj;
        }

        /**
         * Remove object form FaceObjects
         * @maixpy maix.nn.FaceObjects.remove
         */
        err::Err remove(int idx)
        {
            if ((size_t)idx >= objs.size())
                return err::ERR_ARGS;
            objs.erase(objs.begin() + idx);
            return err::ERR_NONE;
        }

        /**
         * Get object item
         * @maixpy maix.nn.FaceObjects.at
         */
        nn::FaceObject &at(int idx)
        {
            return *objs.at(idx);
        }

        /**
         * Get object item
         * @maixpy maix.nn.FaceObjects.__getitem__
         * @maixcdk maix.nn.FaceObjects.operator[]
         */
        nn::FaceObject &operator[](int idx)
        {
            return *objs.at(idx);
        }

        /**
         * Get size
         * @maixpy maix.nn.FaceObjects.__len__
         * @maixcdk maix.nn.FaceObjects.size
         */
        size_t size()
        {
            return objs.size();
        }

        /**
         * Begin
          @maixpy maix.nn.FaceObjects.__iter__
         * @maixcdk maix.nn.FaceObjects.begin
        */
        std::vector<FaceObject*>::iterator begin()
        {
            return objs.begin();
        }

        /**
         * End
         * @maixcdk maix.nn.FaceObjects.end
        */
        std::vector<FaceObject*>::iterator end()
        {
            return objs.end();
        }

    private:
        std::vector<FaceObject *> objs;
    };

    /**
     * FaceRecognizer class
     * @maixpy maix.nn.FaceRecognizer
     */
    class FaceRecognizer
    {
    public:
    public:
        /**
         * Constructor of FaceRecognizer class
         * @param detect_model face detect model path, default empty, you can load model later by load function.
         * @param feature_model feature extract model
         * @param[in] dual_buff prepare dual input output buffer to accelarate forward, that is, when NPU is forwarding we not wait and prepare the next input buff.
         *                      If you want to ensure every time forward output the input's result, set this arg to false please.
         *                      Default true to ensure speed.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.FaceRecognizer.__init__
         * @maixcdk maix.nn.FaceRecognizer.FaceRecognizer
         */
        FaceRecognizer(const string &detect_model = "", const string &feature_model = "", bool dual_buff = true)
        {
            _model_feature = nullptr;
            labels.push_back("unknown");
            _facedetector = nullptr;
            _facedetector_retina = nullptr;
            _facedetector_yolov8 = nullptr;
            _dual_buff = dual_buff;
            if (!detect_model.empty() && !feature_model.empty())
            {
                err::Err e = load(detect_model, feature_model);
                if (e != err::ERR_NONE)
                {
                    throw err::Exception(e, "load face detect model failed");
                }
            }
        }

        ~FaceRecognizer()
        {
            if (_facedetector)
            {
                delete _facedetector;
                _facedetector = nullptr;
            }
            if (_facedetector_retina)
            {
                delete _facedetector_retina;
                _facedetector_retina = nullptr;
            }
            if (_facedetector_yolov8)
            {
                delete _facedetector_yolov8;
                _facedetector_yolov8 = nullptr;
            }
            if (_model_feature)
            {
                delete _model_feature;
                _model_feature = nullptr;
            }
        }

        /**
         * Load model from file
         * @param detect_model face detect model path, default empty, you can load model later by load function.
         * @param feature_model feature extract model
         * @return err::Err
         * @maixpy maix.nn.FaceRecognizer.load
         */
        err::Err load(const string &detect_model, const string &feature_model)
        {
            std::string model_type = _get_detect_model(detect_model);
            if(model_type == "retinaface")
            {
                _facedetector_retina = new Retinaface("", _dual_buff);
                err::Err e = _facedetector_retina->load(detect_model);
                if (e != err::ERR_NONE)
                {
                    log::error("load detect model failed");
                    return e;
                }
                _input_size = _facedetector_retina->input_size();
            }
            else if(model_type == "face_detector")
            {
                _facedetector = new FaceDetector("", _dual_buff);
                err::Err e = _facedetector->load(detect_model);
                if (e != err::ERR_NONE)
                {
                    log::error("load detect model failed");
                    return e;
                }
                _input_size = _facedetector->input_size();
            }
            else if(model_type == "yolov8")
            {
                _facedetector_yolov8 = new YOLOv8("", _dual_buff);
                err::Err e = _facedetector_yolov8->load(detect_model);
                if (e != err::ERR_NONE)
                {
                    log::error("load detect model failed");
                    return e;
                }
                _input_size = _facedetector_yolov8->input_size();
            }
            else
            {
                log::error("model %s not support, only support retinaface and yolov8", model_type.c_str());
                return err::ERR_ARGS;
            }

            // feature extract model
            if (_model_feature)
            {
                delete _model_feature;
                _model_feature = nullptr;
            }
            _model_feature = new nn::NN(feature_model, false);
            if (!_model_feature)
            {
                if(_facedetector)
                {
                    delete _facedetector;
                    _facedetector = nullptr;
                }
                if(_facedetector_retina)
                {
                    delete _facedetector_retina;
                    _facedetector_retina = nullptr;
                }
                if(_facedetector_yolov8)
                {
                    delete _facedetector_yolov8;
                    _facedetector_yolov8 = nullptr;
                }
                return err::ERR_NO_MEM;
            }
            _extra_info2 = _model_feature->extra_info();
            if (_extra_info2.find("model_type") != _extra_info2.end())
            {
                if (_extra_info2["model_type"] != "face_feature")
                {
                    log::error("model_type not match, expect 'face_feature', but got '%s'", _extra_info2["model_type"].c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("model_type key not found");
                return err::ERR_ARGS;
            }
            log::info("model info:\n\ttype: face_feature");
            if (_extra_info2.find("input_type") != _extra_info2.end())
            {
                std::string input_type = _extra_info2["input_type"];
                if (input_type == "rgb")
                {
                    _input_img_fmt = maix::image::FMT_RGB888;
                    log::print("\tinput type: rgb\n");
                }
                else if (input_type == "bgr")
                {
                    _input_img_fmt = maix::image::FMT_BGR888;
                    log::print("\tinput type: bgr\n");
                }
                else
                {
                    log::error("unknown input type: %s", input_type.c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("input_type key not found");
                return err::ERR_ARGS;
            }
            if (_extra_info2.find("mean") != _extra_info2.end())
            {
                std::string mean_str = _extra_info2["mean"];
                std::vector<std::string> mean_strs = split(mean_str, ",");
                log::print("\tmean:");
                for (auto &it : mean_strs)
                {
                    try
                    {
                        this->mean_feature.push_back(std::stof(it));
                    }
                    catch (std::exception &e)
                    {
                        log::error("mean value error, should float");
                        return err::ERR_ARGS;
                    }
                    log::print("%f ", this->mean_feature.back());
                }
                log::print("\n");
            }
            else
            {
                log::error("mean key not found");
                return err::ERR_ARGS;
            }
            if (_extra_info2.find("scale") != _extra_info2.end())
            {
                std::string scale_str = _extra_info2["scale"];
                std::vector<std::string> scale_strs = split(scale_str, ",");
                log::print("\tscale:");
                for (auto &it : scale_strs)
                {
                    try
                    {
                        this->scale_feature.push_back(std::stof(it));
                    }
                    catch (std::exception &e)
                    {
                        log::error("scale value error, should float");
                        return err::ERR_ARGS;
                    }
                    log::print("%f ", this->scale_feature.back());
                }
                log::print("\n");
            }
            else
            {
                log::error("scale key not found");
                return err::ERR_ARGS;
            }
            std::vector<nn::LayerInfo> inputs = _model_feature->inputs_info();
            _feature_input_size = inputs[0].shape[2];
            _std_points = {
                (int)(38.2946f * _feature_input_size / 112),
                (int)(51.6963f * _feature_input_size / 112),
                (int)(73.5318f * _feature_input_size / 112),
                (int)(51.5014f * _feature_input_size / 112),
                (int)(56.0252f * _feature_input_size / 112),
                (int)(71.7366f * _feature_input_size / 112),
                (int)(41.5493f * _feature_input_size / 112),
                (int)(92.3655f * _feature_input_size / 112),
                (int)(70.7299f * _feature_input_size / 112),
                (int)(92.2041f * _feature_input_size / 112),
            };
            return err::ERR_NONE;
        }

        /**
         * Detect objects from image
         * @param img Image want to detect, if image's size not match model input's, will auto resize with fit method.
         * @param conf_th Detect confidence threshold, default 0.5.
         * @param iou_th Detect IoU threshold, default 0.45.
         * @param compare_th Compare two face score threshold, default 0.8, if two faces' score < this value, will see this face fas unknown.
         * @param get_feature return feature or not, if true will copy features to result, if false will not copy feature to result to save time and memory.
         * @param get_face return face image or not, if true result object's face attribute will valid, or face sttribute is empty. Get face image will alloc memory and copy image, so will lead to slower speed.
         * @param fit Resize method, default image.Fit.FIT_CONTAIN.
         * @throw If image format not match model input format, will throw err::Exception.
         * @return FaceObjects object. In C++, you should delete it after use.
         * @maixpy maix.nn.FaceRecognizer.recognize
         */
        nn::FaceObjects *recognize(image::Image &img, float conf_th = 0.5, float iou_th = 0.45, float compare_th = 0.8, bool get_feature = false, bool get_face = false, maix::image::Fit fit = maix::image::FIT_CONTAIN)
        {
            this->_conf_th = conf_th;
            this->_iou_th = iou_th;
            std::vector<nn::Object> *objs;
            nn::Objects *objs2 = nullptr;
            if(_facedetector)
                objs = _facedetector->detect(img, _conf_th, _iou_th, fit);
            else if (_facedetector_retina)
                objs = _facedetector_retina->detect(img, _conf_th, _iou_th, fit);
            else if (_facedetector_yolov8)
                objs2 = _facedetector_yolov8->detect(img, _conf_th, _iou_th, fit);
            FaceObjects *faces = new nn::FaceObjects();
            size_t size = objs2 ? objs2->size() : objs->size();
            for (size_t i = 0; i < size; ++i)
            {
                nn::Object *obj = objs2 ? &objs2->at(i) : &objs->at(i);
                // get std face
                image::Image *std_img = img.affine(obj->points, _std_points, _feature_input_size, _feature_input_size);
                // img.save("/root/test0.jpg");
                // std_img->save("/root/test.jpg");
                tensor::Tensors *outputs = _model_feature->forward_image(*std_img, this->mean_feature, this->scale_feature, fit, false, true);
                if (!outputs) // not ready for dual_buff mode
                {
                    delete std_img;
                    return new FaceObjects();
                }
                tensor::Tensor *out = outputs->tensors[outputs->keys()[0]];
                int fea_len = out->size_int();
                float *feature = (float *)out->data();
                // compare feature from DB
                float max_score = 0;
                int max_i = -1;
                for (size_t i = 0; i < features.size(); ++i)
                {
                    float score = _feature_compare(feature, features[i].data(), fea_len);
                    if (score > max_score)
                    {
                        max_score = score;
                        if(score > compare_th)
                            max_i = i;
                    }
                }
                {
                    faces->add(obj->x, obj->y, obj->w, obj->h, max_i + 1, max_score);
                }
                nn::FaceObject &face1 = faces->at(faces->size() - 1);
                face1.points = obj->points;
                if(get_feature)
                {
                    face1.feature = std::vector<float>(feature, feature + fea_len);
                }
                if(get_face)
                {
                    face1.face = *std_img;
                }
                delete std_img;
                delete outputs;
            }
            return faces;
        }

        /**
         * Add face to lib
         * @param face face object, find by recognize
         * @param label face label(name)
         * @maixpy maix.nn.FaceRecognizer.add_face
         */
        err::Err add_face(nn::FaceObject *face, const std::string &label)
        {
            if (face->feature.empty())
            {
                log::error("face no feature");
                return err::ERR_ARGS;
            }
            labels.push_back(label);
            features.push_back(face->feature);
            return err::ERR_NONE;
        }

        /**
         * remove face from lib
         * @param idx index of face in lib, default -1 means use label, value [0,face_num), idx and label must have one, idx have high priotiry.
         * @param label which face to remove, default to empty string mean use idx, idx and label must have one, idx have high priotiry.
         * @maixpy maix.nn.FaceRecognizer.remove_face
         */
        err::Err remove_face(int idx = -1, const std::string &label = "")
        {
            if (idx == -1 && label.empty())
            {
                log::error("idx and label must have one");
                return err::ERR_ARGS;
            }
            if (!label.empty())
            {
                for (size_t i = 0; i < labels.size(); ++i)
                {
                    if (labels[i] == label)
                    {
                        idx = i - 1;
                        if(idx < 0) // should never < 0
                            throw err::Exception("Code have bug here, idx must >= 0");
                        break;
                    }
                }
            }
            if (idx >= 0 && (size_t)idx < features.size())
            {
                features.erase(features.begin() + idx);
                labels.erase(labels.begin() + idx + 1);
                return err::ERR_NONE;
            }
            log::error("idx value error: %d", idx);
            return err::ERR_ARGS;
        }

        /**
         * Save faces info to a file
         * @param path where to save, string type.
         * @return err.Err type
         * @maixpy maix.nn.FaceRecognizer.save_faces
         */
        err::Err save_faces(const std::string &path)
        {
            std::string dir = fs::dirname(path);
            err::Err e = fs::mkdir(dir);
            if (e != err::ERR_NONE)
            {
                return e;
            }
            fs::File *f = fs::open(path, "w");
            if (!f)
            {
                return err::ERR_IO;
            }
            for (size_t i = 0; i < features.size(); ++i)
            {
                // name + \0 + fea_len(2B) + feature
                f->write(labels[i + 1].c_str(), (int)labels[i + 1].size());
                f->write("\0", 1);
                uint16_t len = (uint16_t)features[i].size();
                f->write(&len, 2);
                f->write(features[i].data(), features[i].size() * sizeof(float));
            }
            f->flush();
            f->close();
            delete f;
            return err::ERR_NONE;
        }

        /**
         * Load faces info from a file
         * @param path from where to load, string type.
         * @return err::Err type
         * @maixpy maix.nn.FaceRecognizer.load_faces
         */
        err::Err load_faces(const std::string &path)
        {
            // Open the file
            fs::File *f = fs::open(path, "r");
            if (!f)
            {
                return err::ERR_IO;
            }

            // Clear current data
            features.clear();
            labels.clear();
            labels.push_back("unknown");

            // Read from the file
            while (!f->eof())
            {
                std::string label;
                char ch;
                // Read label until '\0'
                while (f->read(&ch, 1) && ch != '\0')
                {
                    label += ch;
                }

                // Read the length of the feature vector
                uint16_t len;
                if (f->read(&len, 2) != 2)
                {
                    // Error handling if we cannot read length
                    f->close();
                    delete f;
                    return err::ERR_IO;
                }

                // Read the feature vector
                std::vector<float> feature(len);
                if (f->read(feature.data(), len * sizeof(float)) != (int)(len * sizeof(float)))
                {
                    // Error handling if we cannot read feature data
                    f->close();
                    delete f;
                    return err::ERR_IO;
                }

                // Add the data to the vectors
                labels.push_back(label);
                features.push_back(feature);
            }

            // Close the file and clean up
            f->close();
            delete f;
            return err::ERR_NONE;
        }

        /**
         * Get model input size
         * @return model input size
         * @maixpy maix.nn.FaceRecognizer.input_size
         */
        image::Size input_size()
        {
            return _input_size;
        }

        /**
         * Get model input width
         * @return model input size of width
         * @maixpy maix.nn.FaceRecognizer.input_width
         */
        int input_width()
        {
            return _input_size.width();
        }

        /**
         * Get model input height
         * @return model input size of height
         * @maixpy maix.nn.FaceRecognizer.input_height
         */
        int input_height()
        {
            return _input_size.height();
        }

        /**
         * Get input image format
         * @return input image format, image::Format type.
         * @maixpy maix.nn.FaceRecognizer.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

    public:
        /**
         * Get mean value, list type
         * @maixpy maix.nn.FaceRecognizer.mean_detector
         */
        std::vector<float> mean_detector;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.FaceRecognizer.scale_detector
         */
        std::vector<float> scale_detector;

        /**
         * Get mean value, list type
         * @maixpy maix.nn.FaceRecognizer.mean_feature
         */
        std::vector<float> mean_feature;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.FaceRecognizer.scale_feature
         */
        std::vector<float> scale_feature;

        /**
         * labels, list type, first is "unknown"
         * @maixpy maix.nn.FaceRecognizer.labels
         */
        std::vector<std::string> labels;

        /**
         * features
         * @maixpy maix.nn.FaceRecognizer.features
         */
        std::vector<std::vector<float>> features;

    private:
        image::Size _input_size;
        image::Format _input_img_fmt;
        nn::NN *_model_feature;
        std::map<string, string> _extra_info;
        std::map<string, string> _extra_info2;
        float _conf_th = 0.5;
        float _iou_th = 0.45;
        std::vector<std::vector<float>> _anchor; // [[dense_cx, dense_cy, s_kx, s_ky],]
        std::vector<float> _variance;
        Retinaface *_facedetector_retina;
        YOLOv8 *_facedetector_yolov8;
        FaceDetector *_facedetector;
        int _feature_input_size;
        bool _dual_buff;
        std::vector<int> _std_points;

    private:
        float _feature_compare(float *ftr0, float *ftr1, int len)
        {
            double sumcorr = 0;
            double sumftr0 = 0;
            double sumftr1 = 0;

            for (int i = 0; i < len; i++)
            {
                sumftr0 += ftr0[i] * ftr0[i];
                sumftr1 += ftr1[i] * ftr1[i];
                sumcorr += ftr0[i] * ftr1[i];
            }
            return (0.5 + 0.5 * sumcorr / sqrt(sumftr0 * sumftr1));
        }

        static void split0(std::vector<std::string> &items, const std::string &s, const std::string &delimiter)
        {
            items.clear();
            size_t pos_start = 0, pos_end, delim_len = delimiter.length();
            std::string token;

            while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
            {
                token = s.substr(pos_start, pos_end - pos_start);
                pos_start = pos_end + delim_len;
                items.push_back(token);
            }

            items.push_back(s.substr(pos_start));
        }

        static std::vector<std::string> split(const std::string &s, const std::string &delimiter)
        {
            std::vector<std::string> tokens;
            split0(tokens, s, delimiter);
            return tokens;
        }
    };

} // namespace maix::nn
