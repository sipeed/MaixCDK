/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include "maix_nn_F.hpp"

namespace maix::nn::F
{

    static void _softmax(float *data, int n)
    {
        int stride = 1;
        int i;
        // int diff;
        // float e;
        float sum = 0;
        float largest_i = data[0];

        for (i = 1; i < n; ++i)
        {
            if (data[i * stride] > largest_i)
                largest_i = data[i * stride];
        }
        for (i = 0; i < n; ++i)
        {
            float value = expf(data[i * stride] - largest_i);
            sum += value;
            data[i * stride] = value;
        }
        for (i = 0; i < n; ++i)
        {
            data[i * stride] /= sum;
        }
    }

    tensor::Tensor *softmax(tensor::Tensor *tensor, bool replace)
    {
        if (tensor->dtype() != maix::tensor::DType::FLOAT32)
        {
            throw err::Exception(err::ERR_ARGS, "only support float32 dtype");
        }
        if (replace)
        {
            _softmax((float *)tensor->data(), tensor->size_int());
            return tensor;
        }
        maix::tensor::Tensor *t = new maix::tensor::Tensor(tensor->shape(), tensor->dtype(), tensor->data(), true);
        _softmax((float *)t->data(), t->size_int());
        return t;
    }

} // namespace maix::nn::F
