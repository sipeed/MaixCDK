#include "maix_audio.hpp"
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <vector>  // 用于缓存所有采集的音频数据

// 全局退出标志（信号安全类型）
volatile sig_atomic_t g_quit = 0;

// 全局向量：缓存所有采集的PCM数据（自动管理内存，无需手动扩容）
std::vector<char> g_pcm_cache;

// Ctrl+C信号处理函数
void sigint_handler(int)
{
    g_quit = 1;
}

int main()
{
    // 1. 注册Ctrl+C信号处理
    signal(SIGINT, sigint_handler);

    // 2. 初始化Recorder（16000Hz采样率、FMT_S16_LE对应int16_t、单声道）
    maix::audio::Recorder recorder(
        "",
        16000,
        maix::audio::Format::FMT_S16_LE,
        1,
        true
    );

    // 3. 初始化int16_t缓冲区（16000Hz*100ms=1600个样本）
    const size_t BUF_SIZE = 1600;
    int16_t* data_buf = new int16_t[BUF_SIZE]();

    // 4. 实时采集：仅缓存数据，不写入文件
    while (!g_quit)
    {
        maix::Bytes* pcm = recorder.record(100);
        if (!pcm || pcm->size() == 0) continue;

        // 4.1 拷贝到int16_t缓冲区
        size_t copy_size = std::min(pcm->size(), (size_t)BUF_SIZE * sizeof(int16_t));
        memcpy(data_buf, pcm->data, copy_size);

        // 4.2 缓存到全局向量（后续一次性保存）
        g_pcm_cache.insert(
            g_pcm_cache.end(),
            reinterpret_cast<char*>(data_buf),
            reinterpret_cast<char*>(data_buf) + copy_size
        );

        // 4.3 释放Bytes对象
        delete pcm;
    }

    // 5. 核心：检测到Ctrl+C后，一次性将缓存数据保存为PCM文件
    std::ofstream pcm_file("capture.pcm", std::ios::out | std::ios::binary);
    if (pcm_file.is_open() && !g_pcm_cache.empty())
    {
        pcm_file.write(g_pcm_cache.data(), g_pcm_cache.size());
        pcm_file.close();
    }

    // 6. 清理资源
    delete[] data_buf;
    recorder.finish();

    return 0;
}