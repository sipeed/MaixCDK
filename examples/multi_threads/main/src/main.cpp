
#include "main.h"
#include "maix_thread.hpp"
#include "maix_util.hpp"
#include "maix_basic.hpp"
#include <string>

using namespace maix;

class Params
{
public:
    Params(std::string name, int sleep_ms, int times, bool running)
    {
        this->name = name;
        this->sleep_ms = sleep_ms;
        this->times = times;
        this->running = running;
    }

    std::string name;
    int sleep_ms;
    int times;
    bool running;
};

void run(void *args_in)
{
    Params *args = (Params*)args_in;
    int n = args->times;
    while (n--)
    {
        log::info("[%s] Hello %d", args->name.c_str(), n + 1);
        thread::sleep_ms(1000);
    }
    args->running = false;
}

int _main()
{
    Params args("th0", 1000, 5, true);
    thread::Thread th1(run, &args);
    th1.join();

    Params args2("th1", 1000, 5, true);
    thread::Thread th2(run, &args2);
    th2.detach();
    while(args2.running)
    {
        thread::sleep_ms(2000);
        log::info("main");
    }
    th2.join();
    return 0;
}

int main()
{
    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1);
}
