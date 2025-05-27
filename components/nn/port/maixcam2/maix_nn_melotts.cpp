#include "maix_nn_melotts.hpp"
#include <vector>
#include <string>
#include <array>

namespace maix::nn {
    int MeloTTS::OnnxWrapper::Init(const std::string& model_file) {
        // set ort env
        m_ort_env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, model_file.c_str());
        // 0. session options
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(ORT_ENABLE_ALL);

        // GPU compatiable.
        // OrtCUDAProviderOptions provider_options;
        // session_options.AppendExecutionProvider_CUDA(provider_options);
        // #ifdef USE_CUDA
        //  OrtSessionOptionsAppendExecutionProvider_CUDA(session_options, 0); // C API stable.
        // #endif

        // 1. session
        m_session = new Ort::Session(m_ort_env, model_file.c_str(), session_options);
        // memory allocation and options
        Ort::AllocatorWithDefaultOptions allocator;
        // 2. input name & input dims
        m_input_num = m_session->GetInputCount();
        // for (int i = 0; i < m_input_num; i++) {
        //     std::string input_name(m_session->GetInputNameAllocated(i, allocator).get());
        //     printf("name[%d]: %s\n", i, input_name.c_str());
        // }

        // 4. output names & output dims
        m_output_num = m_session->GetOutputCount();
        // for (int i = 0; i < m_output_num; i++) {
        //     std::string output_name(m_session->GetOutputNameAllocated(i, allocator).get());
        //     printf("name[%d]: %s\n", i, output_name.c_str());
        // }

        return 0;
    }

    std::vector<Ort::Value> MeloTTS::OnnxWrapper::Run(std::vector<int>& phone,
                                    std::vector<int>& tones,
                                    std::vector<int>& langids,
                                    std::vector<float>& g,

                                    float noise_scale,
                                    float noise_scale_w,
                                    float length_scale,
                                    float sdp_ratio) {
        int64_t phonelen = phone.size();
        int64_t toneslen = tones.size();
        int64_t langidslen = langids.size();

        std::array<int64_t, 1> phone_dims{phonelen};
        std::array<int64_t, 3> g_dims{1, 256, 1};
        std::array<int64_t, 1> tones_dims{toneslen};
        std::array<int64_t, 1> langids_dims{langidslen};
        std::array<int64_t, 1> noise_scale_dims{1};
        std::array<int64_t, 1> length_scale_dims{1};
        std::array<int64_t, 1> noise_scale_w_dims{1};
        std::array<int64_t, 1> sdp_scale_dims{1};

        const char* input_names[] = {"phone", "tone", "language", "g", "noise_scale", "noise_scale_w", "length_scale", "sdp_ratio"};
        const char* output_names[] = {"z_p", "pronoun_lens", "audio_len"};

        Ort::MemoryInfo memory_info_handler = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        std::vector<Ort::Value> input_vals;
        input_vals.emplace_back(Ort::Value::CreateTensor<int>(memory_info_handler, phone.data(), phone.size(), phone_dims.data(), phone_dims.size()));
        input_vals.emplace_back(Ort::Value::CreateTensor<int>(memory_info_handler, tones.data(), tones.size(), tones_dims.data(), tones_dims.size()));
        input_vals.emplace_back(Ort::Value::CreateTensor<int>(memory_info_handler, langids.data(), langids.size(), langids_dims.data(), langids_dims.size()));
        input_vals.emplace_back(Ort::Value::CreateTensor<float>(memory_info_handler, g.data(), g.size(), g_dims.data(), g_dims.size()));
        input_vals.emplace_back(Ort::Value::CreateTensor<float>(memory_info_handler, &noise_scale, 1, noise_scale_dims.data(), noise_scale_dims.size()));
        input_vals.emplace_back(Ort::Value::CreateTensor<float>(memory_info_handler, &noise_scale_w, 1, noise_scale_w_dims.data(), noise_scale_w_dims.size()));
        input_vals.emplace_back(Ort::Value::CreateTensor<float>(memory_info_handler, &length_scale, 1, length_scale_dims.data(), length_scale_dims.size()));
        input_vals.emplace_back(Ort::Value::CreateTensor<float>(memory_info_handler, &sdp_ratio, 1, sdp_scale_dims.data(), sdp_scale_dims.size()));

        return m_session->Run(Ort::RunOptions{nullptr}, input_names, input_vals.data(), input_vals.size(), output_names, m_output_num);
    }
}
