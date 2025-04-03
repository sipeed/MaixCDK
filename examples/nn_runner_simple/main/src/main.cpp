
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "main.h"


using namespace maix;

image::Image a = image::Image();
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
    log::info("Program start");
    int ret = 0;
    int repeat_times = 1;

    std::string help = "Usage: " + std::string(argv[0]) + " mud_model_path repeate_times";

    // model path from argv[1], image from argv[2]
    if(argc < 3)
    {
        log::error("args error, %s", help.c_str());
        return -1;
    }
    const char *model_path = argv[1];
    repeat_times = std::stoi(argv[2]);

    uint64_t ts = time::ticks_ms();
    nn::NN model(model_path);
    log::info("load model %s success, time: %d ms", model_path, time::ticks_ms() - ts);

    std::vector<nn::LayerInfo> inputs_info = model.inputs_info();
    std::vector<nn::LayerInfo> outputs_info = model.outputs_info();
    log::info("Inputs:");
    for(auto item : inputs_info)
    {
        log::print("       %s\n", item.to_str().c_str());
    }
    log::info("Outputs:");
    for(auto item : outputs_info)
    {
        log::print("       %s\n", item.to_str().c_str());
    }


    log::info("generate inputs now");
    tensor::Tensors input_tensors;
    for(auto item : inputs_info)
    {
        tensor::Tensor *t = new tensor::Tensor(item.shape, item.dtype);
        input_tensors.add_tensor(item.name, t, false, true);
    }

    tensor::Tensors outputs;
    int count = 3;
    while (repeat_times > 1 && count-- > 0) // warn up
    {
        err::Err e = model.forward(input_tensors, outputs, false);
        if(e != err::Err::ERR_NONE)
        {
            log::error("forward failed");
            goto end;
        }
        outputs.clear();
    }
    ts = time::ticks_ms();
    while (1)
    {
        err::Err e = model.forward(input_tensors, outputs, false);
        if(e != err::Err::ERR_NONE)
        {
            log::error("forward failed");
            goto end;
        }
        ++ count;
        if(count >= repeat_times)
            break;
        outputs.clear();
    }
    log::info("forward success, average time: %d ms", (int)((time::ticks_ms() - ts) / repeat_times));
    for(auto &it : outputs)
    {
        tensor::Tensor *tensor = it.second;
        log::info("======= output '%s':", it.first.c_str());
        log::print("       info: %s\n",(*tensor).to_str().c_str());

        auto names = split(it.first, "/");
        std::string filename = names[names.size() - 1] + ".bin";
        log::print("       save as bin file to '%s'\n", filename.c_str());
        fs::File *f = fs::open(filename, "wb");
        if(!f)
        {
            log::error("open file faild");
            goto end;
        }
        f->write(tensor->data(), tensor->size_int() * sizeof(float));
        delete f;
    }
end:

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


