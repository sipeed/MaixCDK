#include "maix_nn.hpp"
#include "maix_image.hpp"

namespace maix::nn
{
    err::Err maixcam_load_cvimodel(const std::string &model_path, MUD *mud_obj);
    int maix_nn_self_learn_classifier_learn(std::vector<float *> &features, std::vector<float *> &features_samples, int feature_num);

    class NN_MaixCam : public NNBase
    {
    public:
        NN_MaixCam();
        ~NN_MaixCam();

        /**
         * Load model from file
         * @param[in] mud simply parsed model describe object
         * @return error code, if load success, return err::ERR_NONE
         */
        virtual err::Err load(const MUD &mud, const std::string &dir) final;

        /**
         * Unload model
         * @return error code, if unload success, return err::ERR_NONE
         */
        virtual err::Err unload() final;

        /**
         * Unload model
         * @return error code, if unload success, return err::ERR_NONE
         */
        virtual bool loaded() final;

        /**
         * Get model input layer info
         * @return input layer info
         */
        std::vector<LayerInfo> inputs_info();

        /**
         * Get model output layer info
         * @return output layer info
         */
        std::vector<LayerInfo> outputs_info();

        /**
         * forward run model, get output of model
         * @param[in] input input tensor
         * @param[out] output output tensor
         * @return error code, if forward success, return err::ERR_NONE
         */
        virtual err::Err forward(tensor::Tensors &inputs, tensor::Tensors &outputs) final;

        /**
         * forward run model, get output of model,
         * this is specially for MaixPy, not efficient, but easy to use in MaixPy
         * @param[in] input input tensor
         * @return output tensor
         */
        virtual tensor::Tensors *forward(tensor::Tensors &inputs) final;

        /**
         * forward model, param is image
         * @param[in] img input image
         * @return output tensor
         */
        virtual tensor::Tensors *forward_image(image::Image &img, std::vector<float> mean = std::vector<float>(), std::vector<float> scale = std::vector<float>(), image::Fit fit = image::Fit::FIT_CONTAIN, bool copy_result = true) final;

    private:
        bool _loaded;
        void *_data;
    };

} // namespace maix::nn
