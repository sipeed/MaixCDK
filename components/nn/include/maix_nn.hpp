/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#pragma once

#include <map>
#include <vector>
#include <string>
#include "maix_err.hpp"
#include "maix_tensor.hpp"
#include "maix_image.hpp"

namespace maix::nn
{
    /**
     * MUD(model universal describe file) class
     * @maixpy maix.nn.MUD
     */
    class MUD
    {
    public:
        /**
         * MUD constructor
         * @param[in] model_path model file path, model format can be MUD(model universal describe file) file.
         *                       If model_path set, will load model from file, load failed will raise err.Exception.
         *                       If model_path not set, you can load model later by load function.
         * @maixpy maix.nn.MUD.__init__
         * @maixcdk maix.nn.MUD.MUD
         */
        MUD(const char *model_path = nullptr);
        ~MUD();

        /**
         * Load model from file
         * @param[in] model_path model file path, model format can be MUD(model universal describe file) file.
         * @return error code, if load success, return err::ERR_NONE
         * @maixpy maix.nn.MUD.load
         */
        err::Err load(const std::string &model_path);

        /**
         * Model type, string type
         * @maixpy maix.nn.MUD.type
         */
        std::string type;

        /**
         * Model config items, different model type has different config items
         * @maixpy maix.nn.MUD.items
         */
        std::map<std::string, std::map<std::string, std::string>> items;
    };

    /**
     * NN model layer info
     * @maixpy maix.nn.LayerInfo
    */
    class LayerInfo
    {
    public:
        /**
         * LayerInfo constructor
         * @param[in] name layer name
         * @param[in] dtype layer data type
         * @param[in] shape layer shape
         * @maixpy maix.nn.LayerInfo.__init__
         * @maixcdk maix.nn.LayerInfo.LayerInfo
         */
        LayerInfo(const std::string &name =  "", tensor::DType dtype = tensor::DType::FLOAT32, std::vector<int> shape = std::vector<int>())
        {
            this->name = name;
            this->dtype = dtype;
            this->shape = shape;
        }

        /**
         * Layer name
         * @maixpy maix.nn.LayerInfo.name
         */
        std::string   name;

        /**
         * Layer data type
         * @attention If model is quantized, this is the real quantized data type like int8 float16,
         *            in most scene, inputs and outputs we actually use float32 in API like forward.
         * @maixpy maix.nn.LayerInfo.dtype
         */
        tensor::DType dtype;

        /**
         * Layer shape
         * @maixpy maix.nn.LayerInfo.shape
         */
        std::vector<int> shape;

        /**
         * Shape as one int type, multiply all dims of shape
         * @maixpy maix.nn.LayerInfo.shape_int
         */
        int shape_int()
        {
            int n = shape.size() == 0 ? 0 : 1;
            for(auto i : shape)
            {
                n *= i;
            }
            return n;
        }

        /**
         * To string
         * @maixpy maix.nn.LayerInfo.to_str
         */
        std::string to_str()
        {
            std::string str = "LayerInfo(";
            str += "name='";
            str += name;
            str += "', dtype=";
            str += tensor::dtype_name[dtype];
            str += ", shape=[";
            for (size_t i = 0; i < shape.size(); i++)
            {
                str += std::to_string(shape[i]);
                if (i < shape.size() - 1)
                {
                    str += ", ";
                }
            }
            str += "])";
            return str;
        }

        /**
         * To string
         * @maixpy maix.nn.LayerInfo.__str__
         */
        std::string __str__()
        {
            return to_str();
        }
    };

    class NNBase
    {
    public:
        virtual ~NNBase() {}

        /**
         * Load model from file
         * @param mud simply parsed model describe object
         * @param dir directory of model file, always absolute path
         * @return error code, if load success, return err::ERR_NONE
         */
        virtual err::Err load(const MUD &mud, const std::string &dir) = 0;

        /**
         * Unload model
         * @return error code, if unload success, return err::ERR_NONE
         */
        virtual err::Err unload() = 0;

        /**
         * Is model loaded
         * @return true if model loaded, else false
         */
        virtual bool loaded() = 0;

        /**
         * Get model input layer info
         * @return input layer info
         */
        virtual std::vector<LayerInfo> inputs_info() = 0;

        /**
         * Get model output layer info
         * @return output layer info
         */
        virtual std::vector<LayerInfo> outputs_info() = 0;

        /**
         * forward run model, get output of model
         * @param[in] input input tensor
         * @param[out] output output tensor, a key-value dict, key is layer name, value is tensor.
         *                    In C++, value can be allocated by caller, if outputs size is zero, forward will allocate memory, so you should delete it after use.
         * @return error code, if forward success, return err::ERR_NONE
         */
        virtual err::Err forward(tensor::Tensors &inputs, tensor::Tensors &outputs) = 0;

        /**
         * forward run model, get output of model,
         * this is specially for MaixPy, not efficient, but easy to use in MaixPy
         * @param[in] input input tensor
         * @return output tensor, dict type. key is layer name, value is tensor.
         *        In C++, value can be allocated by caller, if outputs size is zero, forward will allocate memory, so you should delete it after use.
         */
        virtual tensor::Tensors *forward(tensor::Tensors &inputs) = 0;

        /**
         * forward model, param is image
         * @param[in] img input image
         * @param fit fit mode, if the image size of input not equal to model's input, it will auto resize use this fit method,
         *           default is image.Fit.FIT_FILL for easy coordinate calculation, but for more accurate result, use image.Fit.FIT_CONTAIN is better.
         * @return output tensor
         */
        virtual tensor::Tensors *forward_image(image::Image &img, std::vector<float> mean = {}, std::vector<float> scale = {}, image::Fit fit = image::Fit::FIT_CONTAIN, bool copy_result = true) = 0;
    };

    /**
     * Neural network class
     * @maixpy maix.nn.NN
     */
    class NN
    {
    public:
        /**
         * Neural network constructor
         * @param[in] model model file path, model format can be MUD(model universal describe file) file.
         *                       If model_path set, will load model from file, load failed will raise err.Exception.
         *                       If model_path not set, you can load model later by load function.
         * @maixpy maix.nn.NN.__init__
         */
        NN(const std::string &model = "");
        ~NN();

        /**
         * Load model from file
         * @param[in] model model file path, model format can be MUD(model universal describe file) file.
         * @return error code, if load success, return err::ERR_NONE
         * @maixpy maix.nn.NN.load
         */
        err::Err load(const std::string &model);

        /**
         * Unload model
         * @return error code, if unload success, return err::ERR_NONE
         */
        err::Err unload();

        /**
         * Is model loaded
         * @return true if model loaded, else false
         * @maixpy maix.nn.NN.loaded
         */
        bool loaded();

        /**
         * Get model input layer info
         * @return input layer info
         * @maixpy maix.nn.NN.inputs_info
         */
        std::vector<nn::LayerInfo> inputs_info();

        /**
         * Get model output layer info
         * @return output layer info
         * @maixpy maix.nn.NN.outputs_info
         */
        std::vector<nn::LayerInfo> outputs_info();

        /**
         * Get model extra info define in MUD file
         * @return extra info, dict type, key-value object, attention: key and value are all string type.
         * @maixpy maix.nn.NN.extra_info
         */
        std::map<std::string, std::string> extra_info();

        /**
         * forward run model, get output of model
         * @param[in] input input tensor
         * @param[out] output output tensor
         * @return error code, if forward success, return err::ERR_NONE
         */
        err::Err forward(tensor::Tensors &inputs, tensor::Tensors &outputs);

        /**
         * forward run model, get output of model,
         * this is specially for MaixPy, not efficient, but easy to use in MaixPy
         * @param[in] input input tensor
         * @return output tensor. In C++, you should manually delete tensors in return value and return value.
         * @maixpy maix.nn.NN.forward
         */
        tensor::Tensors *forward(tensor::Tensors &inputs);

        /**
         * forward model, param is image
         * @param img input image
         * @param mean mean value, a list type, e.g. [0.485, 0.456, 0.406], default is empty list means not normalize.
         * @param scale scale value, a list type, e.g. [1/0.229, 1/0.224, 1/0.225], default is empty list means not normalize.
         * @param fit fit mode, if the image size of input not equal to model's input, it will auto resize use this fit method,
         *           default is image.Fit.FIT_FILL for easy coordinate calculation, but for more accurate result, use image.Fit.FIT_CONTAIN is better.
         * @param copy_result If set true, will copy result to a new variable; else will use a internal memory, you can only use it until to the next forward.
         *                    Default true to avoid problems, you can set it to false manually to make speed faster.
         * @return output tensor. In C++, you should manually delete tensors in return value and return value.
         * @maixpy maix.nn.NN.forward_image
         */
        tensor::Tensors *forward_image(image::Image &img, std::vector<float> mean = std::vector<float>(), std::vector<float> scale = std::vector<float>(), image::Fit fit = image::Fit::FIT_FILL, bool copy_result = true);

    private:
        MUD _mud;
        NNBase *_impl;
    };

}; // namespace maix::nn
