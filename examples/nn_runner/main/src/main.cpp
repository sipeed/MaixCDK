
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "main.h"

using namespace maix;

int post_process_classifier(tensor::Tensors *outputs, const std::string &label_path)
{
    int ret = 0;

    // only support one output
    if((*outputs).size() != 1)
    {
        log::error("only support one output model for classifier");
        return -1;
    }

    // load labels
    std::vector<std::string> labels;
    fs::File *f = fs::open(label_path, "r");
    if(!f)
    {
        log::error("open label file %s failed", label_path);
        return -1;
    }
    std::string line;
    while(f->readline(line) > 0)
    {
        // strip line
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        labels.push_back(line);
    }
    f->close();
    delete f;

    // get top 5 max probability and print
    tensor::Tensor *tensor = outputs->begin()->second;
    std::tuple<tensor::Tensor*, std::vector<int>*> topk = tensor->topk(5);
    log::info("total labels num: %d", labels.size());
    log::info("top 5 result:");
    tensor::Tensor *topk_tensor = std::get<0>(topk);
    std::vector<int> *topk_index = std::get<1>(topk);
    for(size_t i = 0; i < topk_index->size(); i++)
    {
        int index = (*topk_index)[i];
        float value = *((float*)topk_tensor->data() + i);
        log::info("  %d: '%s', %.2f", i, labels[index].c_str(), value);
    }
    printf("\n");
    delete topk_index;
    delete topk_tensor;

    return ret;
}

static std::vector<std::string> split(const std::string &s, const std::string &delimiter) {
    std::vector<std::string> tokens;
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;

    while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        tokens.push_back (token);
    }

    tokens.push_back (s.substr (pos_start));
    return tokens;
}

int _main(int argc, char* argv[])
{
    uint64_t t = time::ticks_ms();
    log::info("Program start");
    std::string model_type = "unknown";
    int ret = 0;
    maix::image::Format img_fmt = maix::image::FMT_RGB888;
    std::vector<float> mean = {};
    std::vector<float> scale = {};
    int forward_loop = 1;

    std::string help = "Usage: " + std::string(argv[0]) + " mud_model_path image_path";

    // model path from argv[1], image from argv[2]
    if(argc < 3)
    {
        log::error("model path not set, %s", help.c_str());
        return -1;
    }
    const char *model_path = argv[1];
    const char *img_path = argv[2];
    // have argv[3], is forward loop test
    if(argc >= 4)
    {
        forward_loop = std::stoi(argv[3]);
        log::info("forward loop times: %d", forward_loop);
    }
    string label_path = "";

    nn::NN model(model_path);
    log::info("load model %s success, time: %d ms", model_path, time::ticks_ms() - t);

    // get model info like preprocess, postprocess info, etc.
    std::map<std::string, std::string> extra_info = model.extra_info();
    if(extra_info.find("model_type") != extra_info.end())
    {
        model_type = extra_info["model_type"];
    }
    if(extra_info.find("input_type") != extra_info.end())
    {
        std::string input_type = extra_info["input_type"];
        if(input_type == "rgb")
        {
            img_fmt = maix::image::FMT_RGB888;
        }
        else if(input_type == "bgr")
        {
            img_fmt = maix::image::FMT_BGR888;
        }
        else
        {
            log::error("unknown input type: %s", input_type.c_str());
            return -1;
        }
    }
    if(extra_info.find("mean") != extra_info.end())
    {
        std::string mean_str = extra_info["mean"];
        std::vector<std::string> mean_strs = split(mean_str, ",");
        for(auto &it : mean_strs)
        {
            mean.push_back(std::stof(it));
        }
    }
    if(extra_info.find("scale") != extra_info.end())
    {
        std::string scale_str = extra_info["scale"];
        std::vector<std::string> scale_strs = split(scale_str, ",");
        for(auto &it : scale_strs)
        {
            scale.push_back(std::stof(it));
        }
    }
    if(extra_info.find("labels") != extra_info.end())
    {
        label_path = fs::dirname(model_path) + "/" + extra_info["labels"];
        log::info("label path: %s", label_path.c_str());
    }

    // printf args info
    log::print("\n");
    log::info("model path: %s", model_path);
    log::info("image path: %s", img_path);
    log::info("model type: %s", model_type.c_str());
    log::info("image format: %s", img_fmt == maix::image::FMT_RGB888 ? "rgb" : "bgr");
    log::info("mean: ");
    for(auto &it : mean)
    {
        log::print("%f, ", it);
    }
    log::print("\n");
    log::info("scale: ");
    for(auto &it : scale)
    {
        log::print("%f, ", it);
    }
    log::print("\n");

    std::vector<nn::LayerInfo> inputs_info = model.inputs_info();
    for(auto &it : inputs_info)
    {
        log::info("input '%s': %s", it.name.c_str(), it.to_str().c_str());
    }

    std::vector<nn::LayerInfo> outputs_info = model.outputs_info();
    for(auto &it : outputs_info)
    {
        log::info("output '%s': %s", it.name.c_str(), it.to_str().c_str());
    }

    log::info("load image now");
    t = time::ticks_ms();
    maix::image::Image *img = maix::image::load(img_path, img_fmt);
    if(!img)
    {
        log::error("load image %s failed", img_path);
        return -1;
    }
    log::info("load image %s success: %s, time: %d ms", img_path, img->to_str().c_str(), time::ticks_ms() - t);

    if(img->width() != inputs_info[0].shape[3] || img->height() != inputs_info[0].shape[2])
    {
        log::warn("image size not match model input size, will auto resize from %dx%d to %dx%d", img->width(), img->height(), inputs_info[0].shape[3], inputs_info[0].shape[2]);
    }
    t = time::ticks_us();
    tensor::Tensors *outputs;
    int count = 0;
    while (1)
    {
        outputs = model.forward_image(*img, mean, scale, maix::image::FIT_COVER);
        if(!outputs)
        {
            log::error("forward image failed");
            goto end;
        }
        ++ count;
        if(count >= forward_loop)
            break;
        delete outputs;
    }
    if(outputs)
    {
        log::info("forward image success, time: %d us", (int)((time::ticks_us() - t) / forward_loop));
        for(auto &it : *outputs)
        {
            tensor::Tensor *tensor = it.second;
            log::info("output '%s': %s", it.first.c_str(), (*tensor).to_str().c_str());
        }
        // post process
        if(model_type == "classifier")
        {
            log::info("post process for classifier model");
            if(label_path.empty())
            {
                log::error("label path not set");
                goto end;
            }
            ret = post_process_classifier(outputs, label_path);
        }
        else
        {
            log::info("no post process for model type: %s, ignore", model_type.c_str());
        }
    }

end:
    if(outputs)
        delete outputs;
    if(img)
        delete img;

    log::info("Program exit");

    return ret;
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}


