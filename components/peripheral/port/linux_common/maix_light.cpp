/**
 * @author Sipeed
 * @copyright Sipeed Ltd 2026-
 * @license Apache 2.0
 * @update 2026.11.5: create this file.
 */

#include "maix_light.hpp"
#include "maix_log.hpp"
#include "maix_sys.hpp"
#include "maix_app.hpp"
#include "maix_pinmap.hpp"
#include "maix_gpio.hpp"
#include "maix_pwm.hpp"
#include <cstdlib>
#include <string>
#include <vector>

#if defined(PLATFORM_MAIXCAM2)
#define LIGHT_DEFAULT_PIN "B25"
#else
#define LIGHT_DEFAULT_PIN "B3"
#endif

namespace maix::peripheral::light
{
    static std::string _str_trim(const std::string &s)
    {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
            return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    static std::string _get_light_pin()
    {
        std::string pin = LIGHT_DEFAULT_PIN;
        try
        {
            auto configs = sys::device_configs();
            auto it = configs.find("cam_light_io");
            if (it != configs.end())
            {
                std::string value = _str_trim(it->second);
                if (!value.empty())
                    pin = value;
            }
        }
        catch (const std::exception &e)
        {
            log::warn("read device configs failed: %s", e.what());
        }
        return pin;
    }

#if defined(PLATFORM_MAIXCAM2)
#define LIGHT_DUTY_WARN_THRESHOLD 70.0

    static void _warn_high_duty(double duty)
    {
        log::warn("light duty %.0f%% is greater than %.0f%%, long time high power running may cause the LED to age quickly",
                  duty, LIGHT_DUTY_WARN_THRESHOLD);
    }
#endif

    struct LightPriv
    {
        bool is_on;
        std::string pin;
        int freq;      // PWM frequency, 0 if driven by GPIO
        double duty;   // configured duty from /boot/configs, used when on() called
        double cur_duty;
#if defined(PLATFORM_MAIXCAM2)
        pwm::PWM *pwm;
#endif
        gpio::GPIO *gpio;

        LightPriv()
            : is_on(false)
            , freq(0)
            , duty(0)
            , cur_duty(0)
#if defined(PLATFORM_MAIXCAM2)
            , pwm(nullptr)
#endif
            , gpio(nullptr)
        {
        }
    };

#if defined(PLATFORM_MAIXCAM2)
    static int _get_pin_pwm_id(const std::string &pin, int &pwm_id)
    {
        try
        {
            std::vector<std::string> funcs = pinmap::get_pin_functions(pin);
            for (auto &func : funcs)
            {
                if (func.rfind("PWM", 0) == 0)
                {
                    pwm_id = atoi(func.c_str() + 3);
                    return 0;
                }
            }
        }
        catch (const std::exception &e)
        {
            log::warn("get pin %s functions failed: %s", pin.c_str(), e.what());
        }
        return -1;
    }
#endif

    Light::Light()
    {
        LightPriv *priv = new LightPriv();
        _priv = priv;

        priv->pin = _get_light_pin();
        bool use_pwm = false;

#if defined(PLATFORM_MAIXCAM2)
        {
            std::string freq_str = app::get_sys_config_kv("light", "freq", "10000");
            std::string duty_str = app::get_sys_config_kv("light", "duty", "50");
            priv->freq = atoi(freq_str.c_str());
            if (priv->freq <= 0)
                priv->freq = 10000;
            priv->duty = atof(duty_str.c_str());
            if (priv->duty < 0)
                priv->duty = 0;
            if (priv->duty > 100)
                priv->duty = 100;
            if (priv->duty > LIGHT_DUTY_WARN_THRESHOLD)
                _warn_high_duty(priv->duty);

            int pwm_id = -1;
            if (_get_pin_pwm_id(priv->pin, pwm_id) == 0)
            {
                err::check_raise(pinmap::set_pin_function(priv->pin, "PWM" + std::to_string(pwm_id)), "set light pin function failed");
                try
                {
                    priv->pwm = new pwm::PWM(pwm_id, priv->freq, 0);
                    use_pwm = true;
                }
                catch (const std::exception &e)
                {
                    log::warn("open light pwm failed: %s, fallback to gpio", e.what());
                    priv->pwm = nullptr;
                }
            }
            else
            {
                log::warn("pin %s has no PWM function, fallback to gpio", priv->pin.c_str());
            }
        }
#endif

        if (!use_pwm)
        {
            priv->freq = 0;
            err::check_raise(pinmap::set_pin_function(priv->pin, "GPIO" + priv->pin), "set light pin function failed");
            priv->gpio = new gpio::GPIO(priv->pin, gpio::Mode::OUT);
            err::check_null_raise(priv->gpio, "open light gpio failed");
            priv->gpio->low();
        }
        priv->is_on = false;
        priv->cur_duty = 0;
        log::info("light pin: %s, freq: %d, duty: %.1f", priv->pin.c_str(), priv->freq, priv->duty);
    }

    Light::~Light()
    {
        LightPriv *priv = (LightPriv *)_priv;
        if (!priv)
            return;
#if defined(PLATFORM_MAIXCAM2)
        if (priv->pwm)
        {
            priv->pwm->duty(0);
            delete priv->pwm;
            priv->pwm = nullptr;
        }
#endif
        if (priv->gpio)
        {
            priv->gpio->low();
            delete priv->gpio;
            priv->gpio = nullptr;
        }
        delete priv;
        _priv = nullptr;
    }

    err::Err Light::on()
    {
        LightPriv *priv = (LightPriv *)_priv;
        if (!priv)
            return err::ERR_RUNTIME;
#if defined(PLATFORM_MAIXCAM2)
        if (priv->pwm)
        {
            if (priv->pwm->duty(priv->duty) < 0)
                return err::ERR_IO;
            priv->cur_duty = priv->duty;
            priv->is_on = true;
            return err::ERR_NONE;
        }
#endif
        if (priv->gpio)
        {
            priv->gpio->high();
            priv->cur_duty = 100;
            priv->is_on = true;
            return err::ERR_NONE;
        }
        return err::ERR_RUNTIME;
    }

    err::Err Light::off()
    {
        LightPriv *priv = (LightPriv *)_priv;
        if (!priv)
            return err::ERR_RUNTIME;
#if defined(PLATFORM_MAIXCAM2)
        if (priv->pwm)
        {
            if (priv->pwm->duty(0) < 0)
                return err::ERR_IO;
            priv->cur_duty = 0;
            priv->is_on = false;
            return err::ERR_NONE;
        }
#endif
        if (priv->gpio)
        {
            priv->gpio->low();
            priv->cur_duty = 0;
            priv->is_on = false;
            return err::ERR_NONE;
        }
        return err::ERR_RUNTIME;
    }

    bool Light::is_on()
    {
        LightPriv *priv = (LightPriv *)_priv;
        if (!priv)
            return false;
        return priv->is_on;
    }

    err::Err Light::set_duty(double duty)
    {
        LightPriv *priv = (LightPriv *)_priv;
        if (!priv)
            return err::ERR_RUNTIME;
        if (duty < 0)
            duty = 0;
        if (duty > 100)
            duty = 100;
#if defined(PLATFORM_MAIXCAM2)
        if (priv->pwm)
        {
            if (duty > LIGHT_DUTY_WARN_THRESHOLD && priv->cur_duty <= LIGHT_DUTY_WARN_THRESHOLD)
                _warn_high_duty(duty);
            if (priv->pwm->duty(duty) < 0)
                return err::ERR_IO;
            priv->cur_duty = duty;
            priv->is_on = duty > 0;
            return err::ERR_NONE;
        }
#endif
        if (priv->gpio)
        {
            if (duty > 0)
            {
                priv->gpio->high();
                priv->cur_duty = 100;
                priv->is_on = true;
            }
            else
            {
                priv->gpio->low();
                priv->cur_duty = 0;
                priv->is_on = false;
            }
            return err::ERR_NONE;
        }
        return err::ERR_RUNTIME;
    }

    double Light::duty()
    {
        LightPriv *priv = (LightPriv *)_priv;
        if (!priv)
            return 0;
        return priv->cur_duty;
    }

    std::string Light::pin()
    {
        LightPriv *priv = (LightPriv *)_priv;
        if (!priv)
            return "";
        return priv->pin;
    }

    int Light::freq()
    {
        LightPriv *priv = (LightPriv *)_priv;
        if (!priv)
            return 0;
        return priv->freq;
    }
} // namespace maix::peripheral::light
