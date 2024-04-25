/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include "maix_basic.hpp"

namespace maix::nn::F
{

    /**
     * Softmax, only support 1D tensor, multi-dimension tensor will be treated as 1D tensor
     * @param tensor input tensor
     * @param replace change input tensor data directly, if not, will create a new tensor
     * @throw If arg error, will raise err.Exception error
     * @return output tensor, if arg replace is true, return the arg tensor's address.
     *         If not replace, return a new object, so In C++, you should delete it manually in this case!
     * @maixpy maix.nn.F.softmax
    */
    tensor::Tensor *softmax(tensor::Tensor *tensor, bool replace);

} // namespace maix::nn::F

