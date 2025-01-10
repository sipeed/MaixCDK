/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <vector>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <array>
#include <numeric>
#include <algorithm>
#include <tuple>
#include <map>
#include <valarray>
#include "maix_log.hpp"
#include "maix_err.hpp"

namespace maix
{
    namespace tensor
    {
        /**
         * Tensor data types
         * @maixpy maix.tensor.DType
         */
        enum DType
        {
            UINT8 = 0,
            INT8,
            UINT16,
            INT16,
            UINT32,
            INT32,
            FLOAT16,
            FLOAT32,
            FLOAT64,
            BOOL,
            // STRING,
            // OBJECT,
            DTYPE_MAX
        };

        /**
         * Tensor data type size in bytes
         * @attention It's a copy of this variable in MaixPy,
         *            so change it in C++ (e.g. update var in hello function) will not take effect the var inMaixPy.
         *            So we add const for this var to avoid this mistake.
         * @maixpy maix.tensor.dtype_size
        */
        const std::vector<int> dtype_size = {
            1, // UINT8
            1, // INT8
            2, // UINT16
            2, // INT16
            4, // UINT32
            4, // INT32
            2, // FLOAT16
            4, // FLOAT32
            8, // FLOAT64
            1, // BOOL
            // 1, // STRING
            // 1, // OBJECT
            0
        };

        /**
         * Tensor data type name
         * @maixpy maix.tensor.dtype_name
        */
        const std::vector<std::string> dtype_name = {
            "uint8",
            "int8",
            "uint16",
            "int16",
            "uint32",
            "int32",
            "float16",
            "float32",
            "float64",
            "bool",
            // "string",
            // "object",
            "invalid"
        };

        /**
         * Tensor class
         * @maixpy maix.tensor.Tensor
         */
        class Tensor
        {
        public:
            Tensor(){
                _shape = {};
                _dtype = DType::FLOAT32;
                _data = nullptr;
                _is_alloc = false;
            }

            /**
             * Tensor constructor
             * @param shape tensor shape, a int list
             * @param dtype tensor element data type, see DType of this module
             * @maixpy maix.tensor.Tensor.__init__
             */
            Tensor(std::vector<int> shape, tensor::DType dtype)
            {
                _shape = shape;
                _dtype = dtype;
                _data = NULL;
                _is_alloc = false;
                int size = 1;
                for (size_t i = 0; i < shape.size(); i++)
                {
                    size *= shape[i];
                }
                if(!_data)
                {
                    _data = malloc(size * dtype_size[dtype]);
                    _is_alloc = true;
                    log::debug("malloc tensor data\n");
                }
                // log::info("new tensor: %p", this);
            }

            /**
             * Tensor constructor
             * @param shape tensor shape, a int list
             * @param dtype tensor element data type, see DType of this module
             * @param data pointer to data content, can be nullptr, it will automatically alloc memory
             *             and detroy it when this object is destroyed
             * @param copy defalt true to alloc new memory and copy from data.
             * @maixcdk maix.tensor.Tensor.Tensor
             */
            Tensor(std::vector<int> shape, tensor::DType dtype, void *data, bool copy = true)
            {
                _shape = shape;
                _dtype = dtype;
                _data = data;
                _is_alloc = false;
                int size = 1;
                for (size_t i = 0; i < shape.size(); i++)
                {
                    size *= shape[i];
                }
                if((!_data) || (data && copy))
                {
                    _data = malloc(size * dtype_size[dtype]);
                    _is_alloc = true;
                    log::debug("malloc tensor data\n");
                    if(data)
                    {
                        memcpy(_data, data, size * dtype_size[dtype]);
                    }
                }
                // log::info("new tensor: %p", this);
            }

            ~Tensor()
            {
                // log::info("free tensor: %p", this);
                if(_is_alloc)
                {
                    log::debug("free tensor data\n");
                    free(_data);
                    _data = nullptr;
                    _is_alloc = false;
                }
            }

            /**
             * To string
             * @maixpy maix.tensor.Tensor.to_str
             */
            std::string to_str()
            {
                std::string str = "Tensor(";
                for (size_t i = 0; i < _shape.size(); i++)
                {
                    str += std::to_string(_shape[i]);
                    if (i < _shape.size() - 1)
                    {
                        str += ", ";
                    }
                }
                str += ", dtype=";
                str += dtype_name[_dtype];
                str += ")";
                return str;
            }

            /**
             * To string
             * @maixpy maix.tensor.Tensor.__str__
             */
            std::string __str__()
            {
                return to_str();
            }

            /**
             * get tensor shape
             * @return tensor shape, a int list
             * @maixpy maix.tensor.Tensor.shape
            */
            std::vector<int> shape() { return _shape; }

            /**
             * expand tensor shape
             * @param axis axis to expand
             * @maixpy maix.tensor.Tensor.expand_dims
            */
            void expand_dims(int axis)
            {
                if (axis < 0)
                {
                    axis = _shape.size() + axis + 1;
                }
                if (axis < 0 || (size_t)axis > _shape.size())
                {
                    log::error("axis out of range\n");
                    return;
                }
                _shape.insert(_shape.begin() + axis, 1);
            }

            /**
             * reshape tensor shape, if size not match, it will throw an err::Exception
             * @param shape new shape
             * @maixpy maix.tensor.Tensor.reshape
            */
            void reshape(std::vector<int> shape)
            {
                int size = 1;
                for (size_t i = 0; i < shape.size(); i++)
                {
                    size *= shape[i];
                }
                if (size != size_int())
                {
                    log::error("reshape size not match\n");
                    throw err::Exception(err::ERR_ARGS);
                }
                _shape = shape;
            }

            /**
             * Flatten tensor shape to 1D
             * @maixpy maix.tensor.Tensor.flatten
            */
            void flatten()
            {
                _shape = {size_int()};
            }

            int size_int()
            {
                if(_shape.size() == 0)
                {
                    return 0;
                }
                int size = 1;
                for (size_t i = 0; i < _shape.size(); i++)
                {
                    size *= _shape[i];
                }
                return size;
            }

            /**
             * get tensor data type
             * @return tensor data type, see DType of this module
             * @maixpy maix.tensor.Tensor.dtype
            */
            tensor::DType  dtype() { return _dtype; }

            void *data() { return _data; }

            /**
             * get tensor data and return a list
             * @return list type data
             * @maixpy maix.tensor.Tensor.to_float_list
            */
            std::valarray<float>* to_float_list()
            {
                return new std::valarray<float>((float*)_data, size_int());
            }

            void operator=(Tensor &t)
            {
                // printf("copy tensor %d, %d\n", _is_alloc, size_int());
                if(_is_alloc && size_int() < t.size_int())
                {
                    free(_data);
                    _data = nullptr;
                }
                else if(!_is_alloc && size_int() !=0 && size_int() < t.size_int())
                {
                    log::error("tensor copy: size not match\n");
                    throw err::Exception(err::ERR_ARGS);
                }
                _shape = t.shape();
                _dtype = t.dtype();
                if (!_data)
                {
                    _data = malloc(t.size_int() * dtype_size[t.dtype()]);
                    _is_alloc = true;
                }
                memcpy(_data, t.data(), t.size_int() * dtype_size[t.dtype()]);
            }


            /**
             * argmax of tensor
             * @param axis By default, the index is into the flattened array, otherwise along the specified axis., wrong axis will throw an err::Exception
             * @return argmax result, you need to delete it after use in C++.
             * @maixpy maix.tensor.Tensor.argmax
            */
           tensor::Tensor *argmax(int axis = 0xffff)
           {
                if(axis != 0xffff)
                {
                        log::error("only support flatten now\n");
                        throw err::Exception(err::ERR_NOT_IMPL);
                }
                int max_idx = _get_argmax0(_dtype, _data, size_int());
                tensor::Tensor *ret = new tensor::Tensor({1}, tensor::DType::INT32);
                int *ret_data = (int *)ret->data();
                ret_data[0] = max_idx;
                return ret;
           }

            /**
             * argmax1, flattened data max index
             * @return argmax result, int type
             * @maixpy maix.tensor.Tensor.argmax1
            */
            int argmax1()
            {
                return _get_argmax0(_dtype, _data, size_int());
            }

            /**
             * TopK value and index from tensor(only support 1D)
             * @param k top k, k must less than tensor size, wrong k will raise an err::Exception
             * @return top k result, tuple type, first is value tensor, second is index list, you need to delete the element pointer after use in C++.
            */
            std::tuple<tensor::Tensor*, std::vector<int>*> topk(int k)
            {
                if(k > size_int())
                {
                    log::error("k > tensor size\n");
                    throw err::Exception(err::ERR_ARGS);
                }

                tensor::Tensor *value = new tensor::Tensor({k}, _dtype);
                std::vector<int> *index = new std::vector<int>(k);

                #define SORT_TOP_K(type) do{\
                    type *data = (type *)_data;\
                    type *value_data = (type *)value->data();\
                    std::vector<int> idx(size_int());\
                    std::iota(idx.begin(), idx.end(), 0);\
                    std::sort(idx.begin(), idx.end(), [&data](int idx_0, int idx_1) {return data[idx_0] > data[idx_1];});\
                    for (int i = 0; i < k; i++)\
                    {\
                        value_data[i] = data[idx[i]];\
                        (*index)[i] = idx[i];\
                    }\
                }while(0)

                switch (_dtype)
                {
                case tensor::DType::FLOAT32:
                    SORT_TOP_K(float);
                    break;
                case tensor::DType::FLOAT64:
                    SORT_TOP_K(double);
                    break;
                case tensor::DType::UINT8:
                    SORT_TOP_K(uint8_t);
                    break;
                case tensor::DType::INT8:
                    SORT_TOP_K(int8_t);
                    break;
                case tensor::DType::UINT16:
                    SORT_TOP_K(uint16_t);
                    break;
                case tensor::DType::INT16:
                    SORT_TOP_K(int16_t);
                    break;
                case tensor::DType::UINT32:
                    SORT_TOP_K(uint32_t);
                    break;
                case tensor::DType::INT32:
                    SORT_TOP_K(int32_t);
                    break;
                default:
                    log::error("not support dtype %d\n", _dtype);
                    throw err::Exception(err::ERR_NOT_IMPL);
                }
                #undef SORT_TOP_K
                return std::make_tuple(value, index);
            }

        private:
            std::vector<int> _shape;
            DType _dtype;
            void *_data;
            bool _is_alloc;

        private:
            template <typename T>
            static int _get_argmax(T dtype, void *data, size_t size)
            {
                T *data_t = (T *)data;
                T max = data_t[0];
                int max_index = 0;
                for (size_t i = 1; i < size; i++)
                {
                    if (data_t[i] > max)
                    {
                        max = data_t[i];
                        max_index = i;
                    }
                }
                return max_index;
            }

            static int _get_argmax0(tensor::DType dtype, void *data, int size)
            {
                int max_idx = -1;
               switch (dtype)
               {
                case tensor::DType::FLOAT32:
                    max_idx = _get_argmax<float>(dtype, data, size);
                    break;
                case tensor::DType::FLOAT16:
                    max_idx = _get_argmax<float>(dtype, data, size);
                    break;
                case tensor::DType::UINT8:
                    max_idx = _get_argmax<uint8_t>(dtype, data, size);
                    break;
                case tensor::DType::INT8:
                    max_idx = _get_argmax<int8_t>(dtype, data, size);
                    break;
                case tensor::DType::UINT16:
                    max_idx = _get_argmax<uint16_t>(dtype, data, size);
                    break;
                case tensor::DType::INT16:
                    max_idx = _get_argmax<int16_t>(dtype, data, size);
                    break;
                case tensor::DType::UINT32:
                    max_idx = _get_argmax<uint32_t>(dtype, data, size);
                    break;
                case tensor::DType::INT32:
                    max_idx = _get_argmax<int32_t>(dtype, data, size);
                    break;
                case tensor::DType::FLOAT64:
                    max_idx = _get_argmax<double>(dtype, data, size);
                    break;
                default:
                    log::error("not support dtype %d\n", dtype);
                    throw err::Exception(err::ERR_NOT_IMPL);
               }
                return max_idx;
            }
        };

        /**
         * Tensors
         * @maixpy maix.tensor.Tensors
        */
        class Tensors
        {
        public:
            /**
             * Constructor of Tensors
             * @maixpy maix.tensor.Tensors.__init__
             * @maixcdk maix.tensor.Tensors.Tensors
            */
            Tensors()
            {
            }

            ~Tensors()
            {
                // log::info("free tensors: %p", this);
                for(auto &item : tensors)
                {
                    if(_auto_delete[item.first])
                    {
                        // log::info("free tensor in ~Tensors: %p", item.second);
                        delete item.second;
                    }
                }
            }

            /**
             * Add tensor
             * @maixpy maix.tensor.Tensors.add_tensor
            */
            void add_tensor(const std::string &key, tensor::Tensor *tensor, bool copy, bool auto_delete)
            {
                if(copy)
                {
                    tensor::Tensor *t = new tensor::Tensor(tensor->shape(), tensor->dtype(), tensor->data(), true);
                    tensors[key] = t;
                    _auto_delete[key] = true;
                }
                else
                {
                    tensors[key] = tensor;
                    _auto_delete[key] = auto_delete;
                }
            }

            /**
             * Remove tensor
             * @maixpy maix.tensor.Tensors.rm_tensor
            */
            void rm_tensor(const std::string &key)
            {
                if(_auto_delete[key])
                {
                    delete tensors[key];
                }
                tensors.erase(key);
            }

            /**
             * Clear tensors
             * @maixpy maix.tensor.Tensors.clear
            */
            void clear()
            {
                auto _keys = keys();
                for(const auto &k : _keys)
                {
                    rm_tensor(k);
                }
            }

            /**
             * Begin of tensors
             * @maixcdk maix.tensor.Tensors.begin
            */
            std::map<std::string, tensor::Tensor*>::iterator begin()
            {
                return tensors.begin();
            }

            /**
             * End of tensors
             * @maixcdk maix.tensor.Tensors.end
            */
            std::map<std::string, tensor::Tensor*>::iterator end()
            {
                return tensors.end();
            }

            /**
             * __next__
             * @maixcdk maix.tensor.Tensors.next
            */
            std::map<std::string, tensor::Tensor*>::iterator next(std::map<std::string, tensor::Tensor*>::iterator it)
            {
                return ++it;
            }

            /**
             * Get tensor by key
             * @maixpy maix.tensor.Tensors.get_tensor
             * @maixcdk maix.tensor.Tensors.get_tensor
            */
            tensor::Tensor &get_tensor(const std::string &key)
            {
                return *tensors[key];
            }

            /**
             * Operator []
             * @maixpy maix.tensor.Tensors.__getitem__
             * @maixcdk maix.tensor.Tensors.operator[]
            */
            tensor::Tensor &operator[](const std::string &key)
            {
                return *tensors[key];
            }

            /**
             * Size
             * @maixpy maix.tensor.Tensors.__len__
             * @maixcdk maix.tensor.Tensors.size
            */
            size_t size()
            {
                return tensors.size();
            }

            /**
             * Get names
             * @maixpy maix.tensor.Tensors.keys
            */
            std::vector<std::string> keys()
            {
                std::vector<std::string> names;
                for(auto &item : tensors)
                {
                    names.push_back(item.first);
                }
                return names;
            }

        public:
            /**
             * Tensors data, dict type
             * @maixpy maix.tensor.Tensors.tensors
            */
            std::map<std::string, tensor::Tensor*> tensors;
        private:
            std::map<std::string, bool> _auto_delete;
        };


    } // namespace tensor
}; // namespace maix
