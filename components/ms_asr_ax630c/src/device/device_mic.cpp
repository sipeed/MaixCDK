#include "device.h"
#include "maix_audio.hpp"
#include "ms_asr.h"
#include "ms_asr_cfg.h"
#include <pthread.h>
#include <vector>

#define DBG_WFILE   0
#define CAPTURE_PERIOD_MS 100  // Capture 100ms at a time, same as working example

static maix::audio::Recorder *recorder = nullptr;
static FILE *fw;
static int mic_sample_rate = AUDIO_RATE;
static int mic_channels = AUDIO_CHANNEL;

// Background capture using vector for dynamic buffer (like the working example)
static pthread_t capture_thread;
static volatile bool capture_running = false;
static pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
static std::vector<int16_t> audio_buffer;  // Dynamic buffer like working example

extern int ms_asr_dbg_flag;

/*****************************MIC public**********************************/
/* Millisecond delay */
void _Sleep(int ms)
{
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = ms * 1000; 
    select(0, NULL, NULL, NULL, &delay);
    return;
}

// Background thread - captures continuously like the working example
static void* capture_thread_func(void* arg)
{
    printf("[CAPTURE] Thread started\n");
    
    while (capture_running) {
        // Record using the same pattern as working example
        maix::Bytes* pcm = recorder->record(CAPTURE_PERIOD_MS);
        
        if (!pcm || pcm->size() == 0) {
            if (pcm) delete pcm;
            usleep(5000);
            continue;
        }
        
        // Convert bytes to int16_t samples
        size_t sample_count = pcm->size() / sizeof(int16_t);
        int16_t* samples = reinterpret_cast<int16_t*>(pcm->data);
        
        // Lock and append to buffer
        pthread_mutex_lock(&data_mutex);
        audio_buffer.insert(audio_buffer.end(), samples, samples + sample_count);
        pthread_mutex_unlock(&data_mutex);
        
        delete pcm;
    }
    
    printf("[CAPTURE] Thread stopped\n");
    return NULL;
}

// Initialize audio device
int mic_init(char* device_name)
{
    try {
        printf("[INIT] Creating recorder (sample_rate=%d, channels=%d)\n", 
               mic_sample_rate, mic_channels);
        
        // Create recorder exactly like working example
        recorder = new maix::audio::Recorder(
            "",  // No file output
            mic_sample_rate,
            maix::audio::Format::FMT_S16_LE,
            mic_channels,
            true  // Blocking mode
        );
        
        if (!recorder) {
            printf("[ERROR] Failed to create recorder\n");
            return -1;
        }

        printf("set sample_rate = %d, real sample_rate = %d\n", 
               mic_sample_rate, recorder->sample_rate());
        
        int period_size = recorder->period_size(-1);
        printf("period_size = %d\n", period_size);
        
        int period_count = recorder->period_count(-1);
        int buffer_frames = period_size * period_count;
        int buffer_time = (buffer_frames * 1000000) / mic_sample_rate;
        printf("max buffer_time:%d\n", buffer_time);
        printf("sample point per frame: %d\n", period_size);
        
        // Clear buffer and reserve enough space
        audio_buffer.clear();
        audio_buffer.reserve(mic_sample_rate * 4);  // Reserve 4 seconds instead of 2
        
        // Start capture thread
        capture_running = true;
        if (pthread_create(&capture_thread, NULL, capture_thread_func, NULL) != 0) {
            printf("[ERROR] Failed to create capture thread\n");
            delete recorder;
            recorder = nullptr;
            return -1;
        }
        
        // Wait for initial buffer fill - IMPORTANT for stable operation
        printf("[INIT] Waiting for buffer to fill (accumulating 2 seconds of audio)...\n");
        _Sleep(2000);  // Increased from 300ms to 2000ms
        
        // Check buffer status
        pthread_mutex_lock(&data_mutex);
        size_t initial_buffer_size = audio_buffer.size();
        pthread_mutex_unlock(&data_mutex);
        printf("[INIT] Initial buffer: %zu samples (%.1f seconds)\n", 
               initial_buffer_size, (float)initial_buffer_size / mic_sample_rate);
        
        if(ms_asr_dbg_flag & DBG_MICRAW) {
            fw = fopen("micraw.pcm", "w");
            if (fw) {
                printf("[INIT] Opened micraw.pcm for debug output\n");
            }
        }
        
        printf("[INIT] Microphone initialization completed\n");
        return 0;
    }
    catch (const std::exception& e) {
        printf("[ERROR] Recorder initialization failed: %s\n", e.what());
        if (recorder) {
            delete recorder;
            recorder = nullptr;
        }
        return -1;
    }
}

// Read samples - behaves like PCM file read
int mic_read(int16_t* data_buf, int len)
{
    if (!recorder || !capture_running) {
        printf("[ERROR] Recorder not initialized or not running\n");
        return -1;
    }
    
    // Wait until we have enough samples
    int wait_iterations = 0;
    const int max_wait_iterations = 1000;  // 10 seconds timeout
    
    while (wait_iterations < max_wait_iterations) {
        pthread_mutex_lock(&data_mutex);
        size_t available = audio_buffer.size();
        pthread_mutex_unlock(&data_mutex);
        
        if (available >= (size_t)len) {
            break;
        }
        
        // Wait for more data
        usleep(10000);  // 10ms
        wait_iterations++;
    }
    
    if (wait_iterations >= max_wait_iterations) {
        printf("[ERROR] Timeout waiting for audio data\n");
        return -1;
    }
    
    // Read from buffer
    pthread_mutex_lock(&data_mutex);
    
    if ((int)audio_buffer.size() >= len) {
        // Copy requested samples
        memcpy(data_buf, audio_buffer.data(), len * sizeof(int16_t));
        
        // Remove consumed samples from front (like reading from file)
        audio_buffer.erase(audio_buffer.begin(), audio_buffer.begin() + len);
        
        pthread_mutex_unlock(&data_mutex);
    } else {
        // Not enough data (shouldn't happen after wait loop)
        int available = audio_buffer.size();
        memcpy(data_buf, audio_buffer.data(), available * sizeof(int16_t));
        audio_buffer.clear();
        pthread_mutex_unlock(&data_mutex);
        
        printf("[WARNING] Buffer starvation! Only got %d samples, requested %d\n", 
               available, len);
        return available;
    }
    
    // Save to debug file if enabled
    if(ms_asr_dbg_flag & DBG_MICRAW) {
        if(fw) {
            fwrite(data_buf, sizeof(int16_t), len, fw);
            fflush(fw);
        }
    }
    
    // Adaptive rate limiting based on buffer health
    // Goal: Balance between responsive and stable
    pthread_mutex_lock(&data_mutex);
    size_t remaining_samples = audio_buffer.size();
    pthread_mutex_unlock(&data_mutex);
    
    // Calculate delay based on buffer level
    int delay_ms;
    if (remaining_samples > 20000) {
        // Buffer very full (>1.25s), can process quickly
        delay_ms = 50;
    } else if (remaining_samples > 10000) {
        // Buffer healthy (>0.625s), normal speed
        delay_ms = 100;
    } else if (remaining_samples > 5000) {
        // Buffer getting low (>0.3s), slow down
        delay_ms = 150;
    } else {
        // Buffer critical (<0.3s), must slow down
        delay_ms = 200;
    }
    
    usleep(delay_ms * 1000);
    
    return len;
}

// Clear buffer
void mic_clear(void)
{
    printf("[CLEAR] Clearing audio buffer\n");
    
    pthread_mutex_lock(&data_mutex);
    audio_buffer.clear();
    pthread_mutex_unlock(&data_mutex);

    if (fw) {
        fclose(fw);
        fw = nullptr;
    }
    
    if(ms_asr_dbg_flag & DBG_MICRAW) {
        fw = fopen("micraw.pcm", "w");   
    }

    return;
}

// Cleanup
void mic_deinit(void)
{
    printf("[DEINIT] Cleaning up\n");
    
    // Stop capture thread
    capture_running = false;
    if (capture_thread) {
        pthread_join(capture_thread, NULL);
    }
    
    if(ms_asr_dbg_flag & DBG_MICRAW) {
        if (fw) {
            fclose(fw);
            fw = nullptr;
        }
    }

    if (recorder) {
        printf("pcm dev deinit\n");
        recorder->finish();
        delete recorder;
        recorder = nullptr;
    }
    
    // Clear buffer
    pthread_mutex_lock(&data_mutex);
    audio_buffer.clear();
    pthread_mutex_unlock(&data_mutex);
    
    return;
}

void mic_test(void)
{
    int16_t buf[768 * 16 * 2];
    int i = 0;
    ms_asr_dbg_flag |= DBG_MICRAW;
    int res = mic_init("hw:0,0");
    printf("mic init res=%d\n", res);
    while(i < 5) {
        int result = mic_read(buf, 768 * 16);
        printf("Read iteration %d: got %d samples\n", i+1, result);
        i++;
    }
    mic_deinit();
    return;
}

asr_device_t dev_mic = {
    .init   = mic_init,
    .read   = mic_read,
    .clear  = mic_clear,
    .deinit = mic_deinit,
};