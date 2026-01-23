/*
* Simplified YOLO Inference using Native AXERA SDK API
* Only preprocessing and inference, no dependency on ax-samples
* Function-based implementation with detailed comments
*/

#include <cstdio>
#include <cstring>
#include <vector>
#include <fstream>
#include <chrono>
#include <numeric>

#include <opencv2/opencv.hpp>
#include <ax_sys_api.h>
#include <ax_engine_api.h>

const int DEFAULT_IMG_H = 640;
const int DEFAULT_IMG_W = 640;
const int DEFAULT_LOOP_COUNT = 100;

/**
 * @brief 获取当前时间戳（微秒）
 * @return 返回当前时间的微秒数
 */
long long get_current_time_us() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

/**
 * @brief 读取文件到内存缓冲区
 * @param path 文件路径
 * @param buffer 输出缓冲区，函数会自动调整大小
 * @return 成功返回true，失败返回false
 */
bool read_file(const std::string& path, std::vector<char>& buffer) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }
    
    // 获取文件大小
    size_t file_size = file.tellg();
    file.seekg(0);
    
    // 调整缓冲区大小并读取
    buffer.resize(file_size);
    return !!file.read(buffer.data(), buffer.size());
}

/**
 * @brief Letterbox图像预处理
 * 保持长宽比缩放图像，不足部分用灰色填充
 * @param src 输入图像
 * @param dst 输出图像
 * @param target_h 目标高度
 * @param target_w 目标宽度
 */
void letterbox(const cv::Mat& src, cv::Mat& dst, int target_h, int target_w) {
    // 计算缩放比例，保持长宽比
    float scale = std::min((float)target_w / src.cols, (float)target_h / src.rows);
    int new_w = src.cols * scale;
    int new_h = src.rows * scale;
    
    // 缩放图像
    cv::Mat resized;
    cv::resize(src, resized, cv::Size(new_w, new_h));
    
    // 创建目标图像，用灰色填充
    dst = cv::Mat(target_h, target_w, CV_8UC3, cv::Scalar(114, 114, 114));
    
    // 计算padding，将缩放后的图像放在中央
    int pad_w = (target_w - new_w) / 2;
    int pad_h = (target_h - new_h) / 2;
    resized.copyTo(dst(cv::Rect(pad_w, pad_h, new_w, new_h)));
}

/**
 * @brief 图像预处理：读取、letterbox、BGR转RGB
 * @param image_file 图像文件路径
 * @param processed 输出的预处理后图像
 * @param target_h 目标高度
 * @param target_w 目标宽度
 * @return 成功返回0，失败返回-1
 */
int preprocess_image(const std::string& image_file, cv::Mat& processed, 
                     int target_h, int target_w) {
    // 读取图像
    cv::Mat img = cv::imread(image_file);
    if (img.empty()) {
        fprintf(stderr, "ERROR: Failed to read image: %s\n", image_file.c_str());
        return -1;
    }
    fprintf(stdout, "Image loaded: %dx%d\n", img.cols, img.rows);
    
    // Letterbox预处理
    letterbox(img, processed, target_h, target_w);
    
    // BGR转RGB（YOLO模型需要RGB格式）
    cv::cvtColor(processed, processed, cv::COLOR_BGR2RGB);
    
    return 0;
}

/**
 * @brief 初始化AXERA系统
 * @return 成功返回0，失败返回-1
 */
int init_axera_system() {
    int ret = AX_SYS_Init();
    if (ret != 0) {
        fprintf(stderr, "ERROR: AX_SYS_Init failed, ret=0x%x\n", ret);
        return -1;
    }
    fprintf(stdout, "AX_SYS_Init success\n");
    return 0;
}

/**
 * @brief 初始化NPU引擎
 * @return 成功返回0，失败返回-1
 */
int init_npu_engine() {
    AX_ENGINE_NPU_ATTR_T npu_attr;
    memset(&npu_attr, 0, sizeof(npu_attr));
    npu_attr.eHardMode = AX_ENGINE_VIRTUAL_NPU_DISABLE;
    
    int ret = AX_ENGINE_Init(&npu_attr);
    if (ret != 0) {
        fprintf(stderr, "ERROR: AX_ENGINE_Init failed, ret=0x%x\n", ret);
        return -1;
    }
    fprintf(stdout, "AX_ENGINE_Init success\n");
    return 0;
}

/**
 * @brief 加载模型文件并创建推理句柄
 * @param model_file 模型文件路径
 * @param handle 输出的模型句柄
 * @return 成功返回0，失败返回-1
 */
int load_model(const std::string& model_file, AX_ENGINE_HANDLE* handle) {
    // 读取模型文件
    std::vector<char> model_buffer;
    if (!read_file(model_file, model_buffer)) {
        fprintf(stderr, "ERROR: Failed to read model file: %s\n", model_file.c_str());
        return -1;
    }
    fprintf(stdout, "Model file loaded, size: %zu bytes\n", model_buffer.size());
    
    // 创建模型句柄
    int ret = AX_ENGINE_CreateHandle(handle, model_buffer.data(), model_buffer.size());
    if (ret != 0) {
        fprintf(stderr, "ERROR: AX_ENGINE_CreateHandle failed, ret=0x%x\n", ret);
        fprintf(stderr, "Possible reasons:\n");
        fprintf(stderr, "  1. Model file format is incorrect (need .joint format)\n");
        fprintf(stderr, "  2. Model was compiled for different chip\n");
        fprintf(stderr, "  3. Model file is corrupted\n");
        fprintf(stderr, "  4. SDK version mismatch\n");
        return -1;
    }
    fprintf(stdout, "AX_ENGINE_CreateHandle success\n");
    
    return 0;
}

/**
 * @brief 创建推理上下文
 * @param handle 模型句柄
 * @return 成功返回0，失败返回-1
 */
int create_context(AX_ENGINE_HANDLE handle) {
    int ret = AX_ENGINE_CreateContext(handle);
    if (ret != 0) {
        fprintf(stderr, "ERROR: AX_ENGINE_CreateContext failed, ret=0x%x\n", ret);
        return -1;
    }
    fprintf(stdout, "AX_ENGINE_CreateContext success\n");
    return 0;
}

/**
 * @brief 准备输入输出缓冲区
 * @param handle 模型句柄
 * @param io_info 模型IO信息（输出）
 * @param io_data 模型IO数据缓冲区（输出）
 * @return 成功返回0，失败返回-1
 */
int prepare_io_buffers(AX_ENGINE_HANDLE handle, AX_ENGINE_IO_INFO_T** io_info, 
                       AX_ENGINE_IO_T* io_data) {
    // 获取模型IO信息
    int ret = AX_ENGINE_GetIOInfo(handle, io_info);
    if (ret != 0) {
        fprintf(stderr, "ERROR: AX_ENGINE_GetIOInfo failed, ret=0x%x\n", ret);
        return -1;
    }
    fprintf(stdout, "Model IO info: %u inputs, %u outputs\n", 
            (*io_info)->nInputSize, (*io_info)->nOutputSize);
    
    // 清空IO数据结构
    memset(io_data, 0, sizeof(AX_ENGINE_IO_T));
    
    // 分配输入缓冲区
    io_data->nInputSize = (*io_info)->nInputSize;
    io_data->pInputs = new AX_ENGINE_IO_BUFFER_T[(*io_info)->nInputSize];
    
    for (uint32_t i = 0; i < (*io_info)->nInputSize; i++) {
        auto& input = io_data->pInputs[i];
        auto& info = (*io_info)->pInputs[i];
        
        input.nSize = info.nSize;
        ret = AX_SYS_MemAlloc(&input.phyAddr, (void**)&input.pVirAddr, 
                            input.nSize, 128, (AX_S8*)"input");
        if (ret != 0) {
            fprintf(stderr, "ERROR: Failed to alloc input buffer[%u], size=%u, ret=0x%x\n", 
                    i, input.nSize, ret);
            return -1;
        }
    }
    fprintf(stdout, "Input buffers allocated\n");
    
    // 分配输出缓冲区
    io_data->nOutputSize = (*io_info)->nOutputSize;
    io_data->pOutputs = new AX_ENGINE_IO_BUFFER_T[(*io_info)->nOutputSize];
    
    for (uint32_t i = 0; i < (*io_info)->nOutputSize; i++) {
        auto& output = io_data->pOutputs[i];
        auto& info = (*io_info)->pOutputs[i];
        
        output.nSize = info.nSize;
        ret = AX_SYS_MemAlloc(&output.phyAddr, (void**)&output.pVirAddr,
                            output.nSize, 128, (AX_S8*)"output");
        if (ret != 0) {
            fprintf(stderr, "ERROR: Failed to alloc output buffer[%u], size=%u, ret=0x%x\n",
                    i, output.nSize, ret);
            return -1;
        }
    }
    fprintf(stdout, "Output buffers allocated\n");
    
    return 0;
}

/**
 * @brief 将预处理后的图像数据拷贝到输入缓冲区
 * @param processed 预处理后的图像
 * @param io_data 模型IO数据缓冲区
 * @return 成功返回0，失败返回-1
 */
int copy_input_data(const cv::Mat& processed, AX_ENGINE_IO_T* io_data) {
    // 检查数据大小是否匹配
    size_t data_size = processed.total() * processed.elemSize();
    if (data_size > io_data->pInputs[0].nSize) {
        fprintf(stderr, "ERROR: Input data size mismatch: %zu > %u\n", 
                data_size, io_data->pInputs[0].nSize);
        return -1;
    }
    
    // 拷贝数据到输入缓冲区
    memcpy(io_data->pInputs[0].pVirAddr, processed.data, data_size);
    fprintf(stdout, "Input data copied, size: %zu bytes\n", data_size);
    
    return 0;
}

/**
 * @brief 预热推理引擎
 * @param handle 模型句柄
 * @param io_data 模型IO数据缓冲区
 * @param warmup_count 预热次数
 * @return 成功返回0，失败返回-1
 */
int warmup_inference(AX_ENGINE_HANDLE handle, AX_ENGINE_IO_T* io_data, int warmup_count) {
    fprintf(stdout, "Warming up (%d iterations)...\n", warmup_count);
    
    for (int i = 0; i < warmup_count; i++) {
        int ret = AX_ENGINE_RunSync(handle, io_data);
        if (ret != 0) {
            fprintf(stderr, "ERROR: Warmup iteration %d failed, ret=0x%x\n", i, ret);
            return -1;
        }
    }
    
    fprintf(stdout, "Warm up completed\n");
    return 0;
}

/**
 * @brief 执行推理并测量时间
 * @param handle 模型句柄
 * @param io_data 模型IO数据缓冲区
 * @param repeat 推理次数
 * @return 成功返回0，失败返回-1
 */
int run_inference(AX_ENGINE_HANDLE handle, AX_ENGINE_IO_T* io_data, int repeat) {
    fprintf(stdout, "Starting inference (%d iterations)...\n", repeat);
    
    std::vector<float> time_costs_us(repeat);
    
    for (int i = 0; i < repeat; i++) {
        // 记录开始时间
        long long start_time = get_current_time_us();
        
        // 执行推理
        int ret = AX_ENGINE_RunSync(handle, io_data);
        if (ret != 0) {
            fprintf(stderr, "ERROR: Inference iteration %d failed, ret=0x%x\n", i, ret);
            return -1;
        }
        
        // 记录结束时间并计算耗时
        long long end_time = get_current_time_us();
        time_costs_us[i] = (float)(end_time - start_time);
    }
    
    // 计算平均时间
    float total_time_us = std::accumulate(time_costs_us.begin(), time_costs_us.end(), 0.0f);
    float avg_time_us = total_time_us / repeat;
    float avg_time_ms = avg_time_us / 1000.0f;
    
    // 输出结果
    fprintf(stdout, "\n========== Inference Time Statistics ==========\n");
    fprintf(stdout, "Average time: %.2f us (%.2f ms)\n", avg_time_us, avg_time_ms);
    fprintf(stdout, "==============================================\n");
    
    return 0;
}

/**
 * @brief 释放IO缓冲区
 * @param io_data 模型IO数据缓冲区
 */
void free_io_buffers(AX_ENGINE_IO_T* io_data) {
    // 释放输入缓冲区
    if (io_data->pInputs) {
        for (uint32_t i = 0; i < io_data->nInputSize; i++) {
            if (io_data->pInputs[i].pVirAddr) {
                AX_SYS_MemFree(io_data->pInputs[i].phyAddr, io_data->pInputs[i].pVirAddr);
            }
        }
        delete[] io_data->pInputs;
        io_data->pInputs = nullptr;
    }
    
    // 释放输出缓冲区
    if (io_data->pOutputs) {
        for (uint32_t i = 0; i < io_data->nOutputSize; i++) {
            if (io_data->pOutputs[i].pVirAddr) {
                AX_SYS_MemFree(io_data->pOutputs[i].phyAddr, io_data->pOutputs[i].pVirAddr);
            }
        }
        delete[] io_data->pOutputs;
        io_data->pOutputs = nullptr;
    }
}

/**
 * @brief 清理AXERA资源
 * @param handle 模型句柄
 */
void cleanup_axera(AX_ENGINE_HANDLE handle) {
    if (handle) {
        AX_ENGINE_DestroyHandle(handle);
    }
    AX_ENGINE_Deinit();
    AX_SYS_Deinit();
}

/**
 * @brief 主函数
 */
int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <model.joint> <image.jpg> [repeat_count]\n", argv[0]);
        return -1;
    }
    
    std::string model_file = argv[1];
    std::string image_file = argv[2];
    int repeat = (argc > 3) ? atoi(argv[3]) : DEFAULT_LOOP_COUNT;
    
    fprintf(stdout, "Model: %s\n", model_file.c_str());
    fprintf(stdout, "Image: %s\n", image_file.c_str());
    fprintf(stdout, "Repeat: %d\n", repeat);
    fprintf(stdout, "--------------------------------------\n");
    
    // 1. 图像预处理
    cv::Mat processed;
    if (preprocess_image(image_file, processed, DEFAULT_IMG_H, DEFAULT_IMG_W) != 0) {
        return -1;
    }
    
    // 2. 初始化AXERA系统
    if (init_axera_system() != 0) {
        return -1;
    }
    
    // 3. 初始化NPU引擎
    if (init_npu_engine() != 0) {
        AX_SYS_Deinit();
        return -1;
    }
    
    // 4. 加载模型
    AX_ENGINE_HANDLE handle = nullptr;
    if (load_model(model_file, &handle) != 0) {
        cleanup_axera(nullptr);
        return -1;
    }
    
    // 5. 创建推理上下文
    if (create_context(handle) != 0) {
        cleanup_axera(handle);
        return -1;
    }
    
    // 6. 准备IO缓冲区
    AX_ENGINE_IO_INFO_T* io_info = nullptr;
    AX_ENGINE_IO_T io_data;
    if (prepare_io_buffers(handle, &io_info, &io_data) != 0) {
        cleanup_axera(handle);
        return -1;
    }
    
    // 7. 拷贝输入数据
    if (copy_input_data(processed, &io_data) != 0) {
        free_io_buffers(&io_data);
        cleanup_axera(handle);
        return -1;
    }
    
    // 8. 预热
    if (warmup_inference(handle, &io_data, 5) != 0) {
        free_io_buffers(&io_data);
        cleanup_axera(handle);
        return -1;
    }
    
    // 9. 执行推理并测量时间
    if (run_inference(handle, &io_data, repeat) != 0) {
        free_io_buffers(&io_data);
        cleanup_axera(handle);
        return -1;
    }
    
    // 10. 清理资源
    free_io_buffers(&io_data);
    cleanup_axera(handle);
    
    fprintf(stdout, "\nProgram completed successfully!\n");
    return 0;
}