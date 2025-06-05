

#include "tokenizer_service_util.hpp"

namespace maix::nn
{
    // 执行命令并返回退出状态
    static int executeCommand(const std::string& command) {
        int status = std::system(command.c_str());
        return WEXITSTATUS(status);  // 提取退出码
    }

    // 检查服务是否处于active状态
    static bool isServiceActive(const std::string& serviceName) {
        std::string cmd = "systemctl is-active --quiet " + serviceName;
        int status = executeCommand(cmd);
        return status == 0;
    }

    // 尝试启动服务
    static bool startService(const std::string& serviceName) {
        std::string cmd = "systemctl start " + serviceName;
        int status = executeCommand(cmd);
        return status == 0;
    }

    err::Err check_start_tokenizer_service(const std::string &url)
    {
        if(url.find("http://127.0.0.1") != std::string::npos ||
           url.find("http://localhost") != std::string::npos
        )
        {
            if(!isServiceActive("llm-tokenizer.service"))
            {
                log::info("llm-tokenizer.service is not active, starting it...");
                if(!startService("llm-tokenizer.service"))
                {
                    log::error("llm-tokenizer.service is not active and start failed, please check by command `systemctl status llm-tokenizer.service`");
                    return err::ERR_RUNTIME;
                }
                time::sleep(2);
                if(!isServiceActive("llm-tokenizer.service"))
                {
                    log::error("llm-tokenizer.service is not active, please check by command `systemctl status llm-tokenizer.service`");
                    return err::ERR_RUNTIME;
                }
            }
        }
        return err::ERR_NONE;
    }

};
