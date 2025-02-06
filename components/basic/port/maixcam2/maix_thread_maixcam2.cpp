#include "maix_thread.hpp"

#define THREAD_USE_PTHREAD 0

#if THREAD_USE_PTHREAD
#include <pthread.h>
#else
#include <thread>
#endif

using namespace std;
namespace maix::thread
{
    Thread::Thread(std::function<void(void*)> func, void *args)
    {
        this->_func = func;
        this->_args = args;
#if THREAD_USE_PTHREAD
        this->_thread = new pthread_t;
        pthread_create((pthread_t *)this->_thread, NULL, (void *(*)(void *))func, args);
#else
        this->_thread = new std::thread(func, args);
#endif
    }

    Thread::~Thread()
    {
        delete (std::thread *)this->_thread;
    }

    void Thread::join()
    {
#if THREAD_USE_PTHREAD
        pthread_join(*(pthread_t *)this->_thread, NULL);
#else
        if (((std::thread *)this->_thread)->joinable())
        {
            ((std::thread *)this->_thread)->join();
        }
#endif
    }

    void Thread::detach()
    {
#if THREAD_USE_PTHREAD
        pthread_detach(*(pthread_t *)this->_thread);
#else
        ((std::thread *)this->_thread)->detach();
#endif
    }

    void sleep_ms(uint32_t ms)
    {
#if THREAD_USE_PTHREAD
        usleep(ms * 1000);
#else
        this_thread::sleep_for(chrono::milliseconds(ms));
#endif
    }

    bool Thread::joinable()
    {
#if THREAD_USE_PTHREAD
        return true;
#else
        return ((std::thread *)this->_thread)->joinable();
#endif
    }

} // namespace maix::thread
