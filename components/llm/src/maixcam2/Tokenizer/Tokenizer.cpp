#include "Tokenizer.hpp"

#include "httplib.h"
#include "http_utils.hpp"
#include "nlohmann/json.hpp"

#include "sample_log.h"
#include "string_utility.hpp"
#include "memory_utils.hpp"

class Tokenizer_Http : public BaseTokenizer
{
private:
    std::shared_ptr<httplib::Client> cli;
    bool _b_bos, _b_eos;

    std::string base_url;

    int bos_id, eos_id;

    std::string uid;

    std::string _tokenizer_type;

public:
    bool Init(std::string model_path, const std::string &tokenizer_type) override
    {
        _tokenizer_type = tokenizer_type;
        base_url = model_path;
        if (!test_connect_http(base_url, 10))
        {
            ALOGE("connect %s failed", base_url.c_str());
            return false;
        }
        else
        {
            ALOGI("connect %s ok", base_url.c_str());
        }

        cli = std::make_shared<httplib::Client>(base_url);
        cli->set_connection_timeout(10);
        cli->set_read_timeout(10);
        cli->set_write_timeout(10);
        cli->set_keep_alive(true);

        if(!getUid())
        {
            ALOGE("get uid failed");
            return false;
        }

        int try_count = 10;
        int count = try_count;
        while (count-- > 0)
        {
            try
            {
                auto ret = cli->Get("/bos_id?uid=" + uid);
                auto rep = ret.value();
                if (rep.status != 200)
                {
                    ALOGE("get bos_id failed, status: %d", rep.status);
                    return false;
                }
                nlohmann::json j = nlohmann::json::parse(rep.body);
                bos_id = j["bos_id"];
                break;
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ALOGE("get bos_id failed, try again %d/%d", count, try_count);
        }

        count = try_count;
        while (count-- > 0)
        {
            try
            {
                auto ret = cli->Get("/eos_id?uid=" + uid);
                auto rep = ret.value();
                if (rep.status != 200)
                {
                    ALOGE("get eos_id failed, status: %d", rep.status);
                    return false;
                }
                nlohmann::json j = nlohmann::json::parse(rep.body);
                eos_id = j["eos_id"];
                break;
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ALOGE("get eos_id failed, try again %d/%d", count, try_count);
        }

        ALOGI("bos_id: %d, eos_id: %d\n", bos_id, eos_id);

        return true;
    }

    bool getUid(int try_count = 10)
    {
        int count = try_count;
        uid = "";
        while (count-- > 0)
        {
            try
            {
                auto ret = cli->Get("/get_uid?tokenizer_type=" + _tokenizer_type);
                auto rep = ret.value();
                if (rep.status != 200)
                {
                    ALOGE("get uid failed, status: %d", rep.status);
                    return false;
                }
                nlohmann::json j = nlohmann::json::parse(rep.body);
                uid = j["uid"];
                ALOGI("uid: %s", uid.c_str());
                break;
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ALOGE("get uid failed, try again %d/%d", count, try_count);
        }
        if (uid.empty())
        {
            ALOGE("get uid failed, uid is empty");
            return false;
        }
        return true;
    }

    bool Reset(std::string system_prompt, std::vector<int> &tokens) override
    {
        nlohmann::json j;
        j["uid"] = uid;
        if (!system_prompt.empty() and system_prompt != "")
        {
            j["system_prompt"] = system_prompt;
        }

        auto ret = cli->Post("/reset", j.dump(), "application/json");
        auto rep = ret.value();
        if (rep.status != 200)
        {
            // 400 and Invalid uid in body
            if(rep.status == 400 && rep.body.find("Invalid uid") != std::string::npos)
            {
                ALOGW("reset failed, Invalid uid, try to get new uid");
                if(!getUid())
                {
                    ALOGE("get new uid failed");
                    return false;
                }
                // retry reset with new uid
                return Reset(system_prompt, tokens);
            }
            ALOGE("reset failed, status: %d", rep.status);
            return false;
        }
        nlohmann::json j_rep = nlohmann::json::parse(rep.body);
        std::vector<int> _token_ids = j_rep["token_ids"];
        tokens = _token_ids;
        return true;
    }

    bool Encode(std::string input, std::string last_reply, std::vector<int> &tokens, std::vector<int> &tokens_diff, bool b_img_prompt) override
    {
        nlohmann::json j;
        j["uid"] = uid;
        j["text"] = input;
        if (!last_reply.empty() and last_reply != "")
        {
            j["last_reply"] = last_reply;
        }

        j["img_prompt"] = b_img_prompt;
        auto ret = cli->Post("/encode", j.dump(), "application/json");
        auto rep = ret.value();
        if (rep.status != 200)
        {
            // 400 and Invalid uid in body
            if(rep.status == 400 && rep.body.find("Invalid uid") != std::string::npos)
            {
                ALOGW("encode failed, Invalid uid, try to get new uid");
                if(!getUid())
                {
                    ALOGE("get new uid failed");
                    return false;
                }
                // retry encode with new uid
                return Encode(input, last_reply, tokens, tokens_diff, b_img_prompt);
            }
            ALOGE("encode failed, status: %d", rep.status);
            return false;
        }
        nlohmann::json j2;
        try
        {
            j2 = nlohmann::json::parse(rep.body);
        }
        catch (const std::exception &e)
        {
            ALOGE("json parse failed: %s", e.what());
            ALOGE("%s", rep.body.c_str());
            return false;
        }

        std::vector<int> _token_ids = j2["token_ids"];
        std::vector<int> _tokens_diff = j2["diff"];

        tokens = _token_ids;
        tokens_diff = _tokens_diff;

        return true;
    }

    std::string Decode(const std::vector<int> input) override
    {
        int cnt = 2;
        std::string out_str = "";
        while (cnt--)
        {
            nlohmann::json j;
            j["token_ids"] = input;
            j["uid"] = uid;
            auto ret = cli->Post("/decode", j.dump(), "application/json");
            auto rep = ret.value();
            if (rep.status != 200)
            {
                // 400 and Invalid uid in body
                if(rep.status == 400 && rep.body.find("Invalid uid") != std::string::npos)
                {
                    ALOGW("decode failed, Invalid uid, try to get new uid");
                    if(!getUid())
                    {
                        ALOGE("get new uid failed");
                        return "";
                    }
                    // retry decode with new uid
                    continue;
                }
                ALOGE("decode failed, status: %d, try again", rep.status);
                ALOGE("%s", rep.body.c_str());
                usleep(1000 * 1000);
                continue;
            }
            try
            {
                nlohmann::json j2 = nlohmann::json::parse(rep.body);
                out_str = j2["text"];
                break;
            }
            catch (const std::exception &e)
            {
                ALOGE("json parse failed: %s, try again", e.what());
                ALOGE("%s", rep.body.c_str());
                usleep(1000 * 1000);
                continue;
            }
        }
        return out_str;
    }

    int GetBosID() override
    {
        return bos_id;
    }

    int GetEosID() override
    {
        return eos_id;
    }
};

std::shared_ptr<BaseTokenizer> CreateTokenizer(TokenizerType type)
{
    switch (type)
    {
    case TKT_HTTP:
        return std::make_shared<Tokenizer_Http>();
    default:
        ALOGE("unknown tokenizer type: %d", type);
        return nullptr;
    }
}