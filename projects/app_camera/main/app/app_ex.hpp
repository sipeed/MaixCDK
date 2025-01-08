#ifndef __APP_EX_HPP__
#define __APP_EX_HPP__

#include <memory>
#include <future>
#include <cmath>
#include <optional>

#include "maix_uart.hpp"
#include "maix_tmc2209.hpp"
#include "maix_gpio.hpp"
#include "maix_pinmap.hpp"
#include "maix_basic.hpp"

#include "lvgl.h"
#include <opencv2/opencv.hpp>

bool almost_eq(float x, float y, float epsilon=std::numeric_limits<float>::epsilon())
{
    return (std::fabs(x-y) < epsilon);
}

/************TMC2209**************/

#define TMC2209_EXIST_DO(...) if (tmc2209_exist()) {__VA_ARGS__}
#define TMC2209_UART_PORT "/dev/ttyS0"
#define TMC2209_UART_ADDR 0x00

struct TMC2209Parm {
    long baud;
    uint16_t micro_step;
    float screw_pitch;
    float speed_mm_s;
    float step_angle;
    bool isr;
    uint8_t hold_curr;
    uint8_t run_curr;

    /* move parm */
    float move_func_1step;
    float move_1step;
};

bool _scan_tmc2209(const char* uartdev=TMC2209_UART_PORT, uint8_t addr=TMC2209_UART_ADDR)
{
    using namespace maix;
    using namespace maix::peripheral::uart;
    auto uart = UART(uartdev, 115200);

    uint8_t data[] = {0x05, addr, 0x00, 0x48};
    if (uart.write(data, std::size(data)) < 0) {
        return false;
    }
    uint8_t buff[std::size(data)];
    if (uart.read(buff, std::size(buff), -1, 0) < 0) {
        return false;
    }
    for (size_t i = 0; i < std::size(data); ++i) {
        if (data[i] != buff[i]) {
            return false;
        }
    }
    return true;
}

inline static std::unique_ptr<maix::ext_dev::tmc2209::ScrewSlide> g_slide;
inline static TMC2209Parm* g_slide_cfg;
inline static bool g_slide_exist = false;

void _tmc2209_reset(const char* uartdev, uint8_t addr, const TMC2209Parm& parm)
{
    g_slide.reset(nullptr);
    g_slide.reset(new maix::ext_dev::tmc2209::ScrewSlide(uartdev, addr, parm.baud,
        parm.step_angle, parm.micro_step, parm.screw_pitch, parm.speed_mm_s, parm.isr, parm.run_curr, parm.hold_curr));
}

bool tmc2209_init(const char* uartdev=TMC2209_UART_PORT, uint8_t addr=TMC2209_UART_ADDR)
{
    if (!_scan_tmc2209(uartdev, addr))
        return false;
    g_slide_exist = true;
    return true;
}

bool tmc2209_exist()
{
    return g_slide_exist;
}



void tmc2209_2um8_init(const char* uartdev=TMC2209_UART_PORT, uint8_t addr=TMC2209_UART_ADDR)
{
    static TMC2209Parm parm{
        .baud = 115200,
        .micro_step = 256,
        .screw_pitch = 8,
        .speed_mm_s = 0.8,
        .step_angle = 2.0,
        .isr = true,
        .hold_curr = 80,
        .run_curr = 100,
        .move_func_1step = 0.0224,
        .move_1step = 0.0028,
    };
    _tmc2209_reset(uartdev, addr, parm);
    g_slide_cfg = &parm;
}

void tmc2209_22um4_init(const char* uartdev=TMC2209_UART_PORT, uint8_t addr=TMC2209_UART_ADDR)
{
    static TMC2209Parm parm{
        .baud = 115200,
        .micro_step = 256,
        .screw_pitch = 1,
        .speed_mm_s = 0.8,
        .step_angle = 2.0,
        .isr = true,
        .hold_curr = 80,
        .run_curr = 100,
        .move_func_1step = 0.0224,
        .move_1step = 0.0224,
    };
    _tmc2209_reset(uartdev, addr, parm);
    g_slide_cfg = &parm;
}


void tmc2209_1mm_init(const char* uartdev=TMC2209_UART_PORT, uint8_t addr=TMC2209_UART_ADDR)
{
    static TMC2209Parm parm{
        .baud = 115200,
        .micro_step = 256,
        .screw_pitch = 1,
        .speed_mm_s = 1,
        .step_angle = 2.0,
        .isr = true,
        .hold_curr = 80,
        .run_curr = 100,
        .move_func_1step = 1,
        .move_1step = 1,
    };
    _tmc2209_reset(uartdev, addr, parm);
    g_slide_cfg = &parm;
}

void tmc2209_2mm_init(const char* uartdev=TMC2209_UART_PORT, uint8_t addr=TMC2209_UART_ADDR)
{
    static TMC2209Parm parm{
        .baud = 115200,
        .micro_step = 256,
        .screw_pitch = 1,
        .speed_mm_s = 2,
        .step_angle = 2.0,
        .isr = true,
        .hold_curr = 80,
        .run_curr = 100,
        .move_func_1step = 2,
        .move_1step = 2,
    };
    _tmc2209_reset(uartdev, addr, parm);
    g_slide_cfg = &parm;
}

struct TMC2209Status {
    bool run;
    float step_len;
    int steps;
    bool up;
};

constexpr TMC2209Status __low_speed_2um8x4 {
    .run = true,
    .step_len = 0.0028,
    .steps = 4,
    .up = true,
};

constexpr TMC2209Status __low_speed_22um4x1 {
    .run = true,
    .step_len = 0.0224,
    .steps = 1,
    .up = true,
};

constexpr TMC2209Status __high_speed_1mmx1 {
    .run = true,
    .step_len = 1.0,
    .steps = 1,
    .up = true,
};

constexpr TMC2209Status __high_speed_2mmx1 {
    .run = true,
    .step_len = 1,
    .steps = 1,
    .up = true,
};

inline static TMC2209Status g_tmc2209_status = __low_speed_2um8x4;

void tmc2209_init_with(const TMC2209Status& s, const char* uartdev=TMC2209_UART_PORT, uint8_t addr=TMC2209_UART_ADDR)
{
    if (almost_eq(s.step_len, __low_speed_2um8x4.step_len) && s.steps == __low_speed_2um8x4.steps) {
        tmc2209_2um8_init(uartdev, addr);
        g_tmc2209_status.step_len = __low_speed_2um8x4.step_len;
        g_tmc2209_status.steps = __low_speed_2um8x4.steps;
    } else if (almost_eq(s.step_len, __low_speed_22um4x1.step_len) && s.steps == __low_speed_22um4x1.steps) {
        tmc2209_22um4_init(uartdev, addr);
        g_tmc2209_status.step_len = __low_speed_22um4x1.step_len;
        g_tmc2209_status.steps = __low_speed_22um4x1.steps;
    } else if (almost_eq(s.step_len, __high_speed_1mmx1.step_len) && s.steps == __high_speed_1mmx1.steps) {
        tmc2209_1mm_init(uartdev, addr);
        g_tmc2209_status.step_len = __high_speed_1mmx1.step_len;
        g_tmc2209_status.steps = __high_speed_1mmx1.steps;
    } else if (almost_eq(s.step_len, __high_speed_2mmx1.step_len) && s.steps == __high_speed_2mmx1.steps) {
        tmc2209_2mm_init(uartdev, addr);
        g_tmc2209_status.step_len = __high_speed_2mmx1.step_len;
        g_tmc2209_status.steps = __high_speed_2mmx1.steps;
    } else {
        maix::log::error("Unknown init parm");
    }
}

extern "C" {

bool tmc2209_exist_c(void)
{
    return g_slide_exist;
}

}

/****************HP SHOT TP*******************/

inline static std::unique_ptr<maix::peripheral::gpio::GPIO> g_hp_shot;
inline static std::future<void> g_hp_shot_fut;

bool hp_shot_init()
{
    /* gpio a15 */
    using namespace maix::peripheral::gpio;
    using namespace maix::peripheral::pinmap;

    if (set_pin_function("A15", "GPIOA15") < 0) {
        return false;
    }
    g_hp_shot.reset(new GPIO("A15", Mode::OUT, Pull::PULL_NONE));
    g_hp_shot->value(0);
    g_hp_shot_fut = std::async(std::launch::async, [](){});
    return true;
}

void hp_shot_trigger()
{
    if (!g_hp_shot) return;

    // g_hp_shot_fut.wait();

    g_hp_shot->value(1);
    // maix::time::sleep_us(500);
    // g_hp_shot->value(0);
    g_hp_shot_fut = std::async(std::launch::async, [](){
        maix::time::sleep_ms(5);
        g_hp_shot->value(0);
    });
}


/****************SNAP TP*******************/

inline static std::unique_ptr<maix::peripheral::gpio::GPIO> g_snap;
inline static int g_snap_prev_value;

bool snap_init()
{
    using namespace maix::peripheral::pinmap;
    using namespace maix::peripheral::gpio;

    if (set_pin_function("A27", "GPIOA27") < 0) {
        return false;
    }

    g_snap.reset(new GPIO("A27", Mode::IN));
    g_snap_prev_value = g_snap->value();
    return true;
}

bool snap_check(int value)
{
    if (!g_snap) return false;

    int curr_val = g_snap->value();
    if (curr_val != g_snap_prev_value) {
        g_snap_prev_value = curr_val;
        if (value == curr_val) {
            return true;
        }
    }

    return false;
}


/*************LVGL CB****************/

inline static std::optional<lv_point_t> g_auto_focus{std::nullopt};
extern lv_obj_t* g_stack_setting_bar;
extern lv_obj_t* g_stack_shot_number_setting_bar;

struct UIStackStatus {
    int setting_bar_is_hidden;
    int shot_number_setting_bar_is_hidden;
    int is_set_start_point;
    int is_set_end_point;
    int wait_time_ms;


    float move_len;
    int shot_number;
    int shot_number_max;
    int other_camera_mode;
    int reset_at_end_mode;
    int need_reset;
    float reset_len;
    int run;
    int steps_2um8;
    int shot_steps_cnt;

    int can_run;

    decltype(UIStackStatus::move_len) get_move_len() {
        if (!this->is_set_start_point && this->is_set_end_point) {
            return this->move_len;
        }
        return std::nanf("");
        // return make_error_code(err::Err::ERR_NOT_READY);
    }
    std::error_code update(float min_step_len_um) {
        // auto len = this->get_move_len();
        // if (!len) {
        //     return len.error();
        // }
        auto len = this->get_move_len();
        if (isnan(len)){
            return make_error_code(std::errc::invalid_argument);
        }

        // maix::log::info("[Stack] move len: %f", this->move_len);

        float curr_move_len = std::fabs(static_cast<float>(len*1000));
        float curr_shot_number_max = curr_move_len / min_step_len_um;
        this->shot_number_max = static_cast<int>(static_cast<int>(curr_shot_number_max)/10) * 10;
        if (this->shot_number_max < 0) {
            this->shot_number_max = 0;
        }
        if (this->shot_number_max > 990) {
            this->shot_number_max = 990;
        }

        return make_error_code(std::errc{});
    }
    int check() {
        if (/*this->move_len > 0 && */this->shot_number <= this->shot_number_max) {
            can_run = 1;
        } else {
            can_run = 0;
        }
        return can_run;
    }
};

inline static UIStackStatus g_ui_stack_status;

extern "C" {

void ui_stack_status_init(void)
{
    g_ui_stack_status.setting_bar_is_hidden = 1;
    g_ui_stack_status.shot_number_setting_bar_is_hidden = 1;
    g_ui_stack_status.is_set_start_point = 0;
    g_ui_stack_status.move_len = 0;
    g_ui_stack_status.shot_number = 0;
    g_ui_stack_status.shot_number_max = 0;
    g_ui_stack_status.wait_time_ms = 100;
    g_ui_stack_status.other_camera_mode = 0;
    g_ui_stack_status.reset_at_end_mode = 1;
    g_ui_stack_status.run = 0;
    g_ui_stack_status.can_run = 0;
    g_ui_stack_status.need_reset = 0;
    g_ui_stack_status.reset_len = 0;
}

void ui_stack_status_reset(void)
{
    g_ui_stack_status.need_reset = 1;
    /* 不回归起始点,清除起始点结束点长度 */
    if (!g_ui_stack_status.reset_at_end_mode) {
        g_ui_stack_status.move_len = 0;
        g_ui_stack_status.need_reset = 0;
    }
    g_ui_stack_status.run = 0;
    g_ui_stack_status.can_run = 0;
}

void reset_stack_start_btn(void)
{
    lv_obj_t* obj = (lv_obj_t*)lv_obj_get_user_data(g_stack_setting_bar);
    lv_obj_remove_state(obj, LV_STATE_DISABLED);
    lv_obj_remove_state(obj, LV_STATE_CHECKED);
    lv_obj_t* label = (lv_obj_t*)lv_obj_get_user_data(obj);
    lv_label_set_text(label, "start");
}

void update_stack_start_btn(void)
{
    lv_obj_t* obj = (lv_obj_t*)lv_obj_get_user_data(g_stack_setting_bar);
    if (g_ui_stack_status.run) { /* running, never disable */
        lv_obj_remove_state(obj, LV_STATE_DISABLED);
        return;
    }
    if (g_ui_stack_status.check()) {
        lv_obj_remove_state(obj, LV_STATE_DISABLED);
    } else {
        lv_obj_add_state(obj, LV_STATE_DISABLED);
    }
}

void event_tmc2209_up_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED) {
        g_tmc2209_status = __low_speed_2um8x4;
        // tmc2209_init_with(__low_speed_2um8x4);
    } else if (code == LV_EVENT_LONG_PRESSED_REPEAT) {
        g_tmc2209_status = __high_speed_1mmx1;
        // tmc2209_init_with(__high_speed_1mmx1);
    } else if (code == LV_EVENT_RELEASED) {
        g_tmc2209_status.run = false;
    }
    g_tmc2209_status.up = true;
}

void event_tmc2209_down_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED) {
        g_tmc2209_status = __low_speed_2um8x4;
        // tmc2209_init_with(__low_speed_2um8x4);
    } else if (code == LV_EVENT_LONG_PRESSED_REPEAT) {
        g_tmc2209_status = __high_speed_1mmx1;
        // tmc2209_init_with(__high_speed_1mmx1);
    } else if (code == LV_EVENT_RELEASED) {
        g_tmc2209_status.run = false;
    }
    // maix::log::info("set down");
    g_tmc2209_status.up = false;
}

void event_auto_focus_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_point_t point;
        lv_indev_get_point(lv_indev_get_act(), &point);
        g_auto_focus = point;
    }
}

void event_stack_shot_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_ui_stack_status.setting_bar_is_hidden) {
            lv_obj_remove_flag(g_stack_setting_bar, LV_OBJ_FLAG_HIDDEN);
            g_ui_stack_status.setting_bar_is_hidden = 0;
        } else {
            lv_obj_add_flag(g_stack_setting_bar, LV_OBJ_FLAG_HIDDEN);
            g_ui_stack_status.setting_bar_is_hidden = 1;
        }
        if (!g_ui_stack_status.shot_number_setting_bar_is_hidden) {
            lv_obj_add_flag(g_stack_shot_number_setting_bar, LV_OBJ_FLAG_HIDDEN);
            g_ui_stack_status.shot_number_setting_bar_is_hidden = 1;
        }
    }
}

void event_stack_set_start_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        maix::log::info("[Stack] Set start point");
        g_ui_stack_status.is_set_start_point = 1;
        g_ui_stack_status.is_set_end_point = 0;
        g_ui_stack_status.move_len = 0;
        lv_obj_t* obj = lv_event_get_target_obj(e);
        lv_obj_add_state(obj, LV_STATE_CHECKED);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_state(obj, LV_STATE_DISABLED);
        lv_obj_t* obj2 = (lv_obj_t*)lv_obj_get_user_data(obj);
        lv_obj_add_flag(obj2, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_state(obj2, LV_STATE_DISABLED);
    }
}

void event_stack_set_end_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED && g_ui_stack_status.is_set_start_point) {
        maix::log::info("[Stack] Set end point");
        g_ui_stack_status.is_set_start_point = 0;
        g_ui_stack_status.is_set_end_point = 1;
        lv_obj_t* obj = lv_event_get_target_obj(e);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_state(obj, LV_STATE_DISABLED);
        lv_obj_t* obj2 = (lv_obj_t*)lv_obj_get_user_data(obj);
        lv_obj_remove_state(obj2, LV_STATE_CHECKED);
        lv_obj_add_flag(obj2, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_state(obj2, LV_STATE_DISABLED);

        g_ui_stack_status.update(__low_speed_2um8x4.step_len*1000);
        update_stack_start_btn();
    }
}

void event_stack_set_shot_number_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        maix::log::info("[Stack] Shot number cb");
        if (g_ui_stack_status.shot_number_setting_bar_is_hidden) {
            lv_obj_remove_flag(g_stack_shot_number_setting_bar, LV_OBJ_FLAG_HIDDEN);
            g_ui_stack_status.shot_number_setting_bar_is_hidden = 0;
        } else {
            lv_obj_add_flag(g_stack_shot_number_setting_bar, LV_OBJ_FLAG_HIDDEN);
            g_ui_stack_status.shot_number_setting_bar_is_hidden = 1;
        }
    }
}

void event_stack_shot_number_change_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t * slider = (lv_obj_t*)lv_event_get_target(e);
        int value = (int)lv_slider_get_value(slider);
        g_ui_stack_status.shot_number = (value*g_ui_stack_status.shot_number_max)/100/10;
        g_ui_stack_status.shot_number *= 10;
        if (g_ui_stack_status.shot_number <= 0) {
            g_ui_stack_status.shot_number = 0;
        }
        if (g_ui_stack_status.shot_number > g_ui_stack_status.shot_number_max) {
            g_ui_stack_status.shot_number = g_ui_stack_status.shot_number_max;
        }
        lv_obj_t* label = (lv_obj_t*)lv_obj_get_user_data(slider);
        lv_label_set_text_fmt(label, "num\n%d", g_ui_stack_status.shot_number);
        lv_obj_center(label);
        // maix::log::info("[Stack] Value change: %d", g_ui_stack_status.shot_number);
    }
    update_stack_start_btn();
}

void event_stack_wait_time_cb(lv_event_t* e)
{
    /**
     * @brief 稳定时间：移动到位置后等待X秒等待平台稳定后拍照
        1. 进度条拖动或步进：0.1,0.2,0.5,1,2,5,10
     *
     */
    static int wait_time_ms_list[] = {
        100, 200, 500, 1000, 2000, 5000, 10000
    };
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
        lv_obj_t* label = (lv_obj_t*)lv_obj_get_user_data(obj);
        bool _flag = false;
        for (decltype(std::size(wait_time_ms_list)) i = 0; i < std::size(wait_time_ms_list) - 1; ++i) {
            if (wait_time_ms_list[i] == g_ui_stack_status.wait_time_ms) {
                g_ui_stack_status.wait_time_ms = wait_time_ms_list[i+1];
                _flag = true;
                break;
            }
        }
        if (!_flag) {
            g_ui_stack_status.wait_time_ms = wait_time_ms_list[0];
            maix::log::info("[Stack] reset wait time to %d", g_ui_stack_status.wait_time_ms);
        }
        float f = (float)g_ui_stack_status.wait_time_ms / 1000.0;
        lv_label_set_text_fmt(label, "wait\n%0.1fs", f);
    }
    update_stack_start_btn();
}

void event_stack_other_camera_mode_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        g_ui_stack_status.other_camera_mode = g_ui_stack_status.other_camera_mode ? 0 : 1;
        lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
        if (g_ui_stack_status.other_camera_mode) {
            lv_obj_add_state(obj, LV_STATE_CHECKED);
        } else {
            lv_obj_remove_state(obj, LV_STATE_CHECKED);
        }
    }
    update_stack_start_btn();
}

void event_stack_reset_at_end_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        g_ui_stack_status.reset_at_end_mode = g_ui_stack_status.reset_at_end_mode ? 0 : 1;
        lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
        if (g_ui_stack_status.reset_at_end_mode) {
            lv_obj_add_state(obj, LV_STATE_CHECKED);
        } else {
            lv_obj_remove_state(obj, LV_STATE_CHECKED);
        }
    }
    update_stack_start_btn();
}

void event_stack_run_or_stop_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
    lv_obj_t* label = (lv_obj_t*)lv_obj_get_user_data(obj);
    if (code == LV_EVENT_CLICKED) {
        if (g_ui_stack_status.run == 0) {
            g_ui_stack_status.steps_2um8 = static_cast<int>(g_ui_stack_status.move_len*1000/0.0028);
            g_ui_stack_status.shot_steps_cnt = g_ui_stack_status.steps_2um8 / g_ui_stack_status.shot_number;
            // maix::log::info("[Stack] steps %d, shot step cnt: %d", g_ui_stack_status.steps_2um8, g_ui_stack_status.shot_steps_cnt);
            g_ui_stack_status.run = 1;
            lv_label_set_text(label, "stop");
            lv_obj_add_flag(g_stack_setting_bar, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_stack_shot_number_setting_bar, LV_OBJ_FLAG_HIDDEN);
        } else {
            g_ui_stack_status.run = 0;
            ui_stack_status_reset();
            lv_label_set_text(label, "start");
            lv_obj_add_state(obj, LV_STATE_DISABLED);
            reset_stack_start_btn();
        }
    }
}

}


/**********AF************/

class ImageClarityEvaluationMethod {
public:
    virtual void crop(cv::Mat& mat, int x, int y) {
        if (mat.empty()) {
            maix::log::error("crop empty return");
            return;
        }

        int width = mat.cols;
        int height = mat.rows;

        // 设定裁剪区域的宽度和高度
        int cropWidth = width / 2;
        int cropHeight = height / 2;

        // 计算裁剪区域的起始坐标，避免越界
        int startX = std::max(0, x - cropWidth / 2);
        int startY = std::max(0, y - cropHeight / 2);

        // 确保裁剪区域在图像内部
        if (startX + cropWidth > width) {
            startX = width - cropWidth;
        }
        if (startY + cropHeight > height) {
            startY = height - cropHeight;
        }

        // 定义裁剪区域
        cv::Rect cropRegion(startX, startY, cropWidth, cropHeight);

        // 裁剪图像
        mat = mat(cropRegion);

        maix::log::info("crop %dx%d --> %dx%d (%d, %d)", width, height, mat.cols, mat.rows, startX, startY);
    }

    virtual void to_gray(cv::Mat& mat) {
        if (mat.empty()) {
            return;
        }

        if (mat.channels() == 3 || mat.channels() == 4) {
            cv::cvtColor(mat, mat, cv::COLOR_BGR2GRAY);
        }
    }

    virtual float clarity(cv::Mat& mat, int x, int y) = 0;
    virtual ~ImageClarityEvaluationMethod() {}
};

// class Tenegrad final : public ImageClarityEvaluationMethod {
// public:
//     virtual void crop(cv::Mat& mat, int x, int y) override {
//         ImageClarityEvaluationMethod::crop(mat, x, y);
//     }
//     virtual void to_gray(cv::Mat& mat) override {
//         ImageClarityEvaluationMethod::to_gray(mat);
//     }
//     virtual float clarity(cv::Mat& mat, int x, int y) override {
//         this->crop(mat, x, y);
//         this->to_gray(mat);

//         cv::Mat xgrad;
// 		cv::Mat ygrad;
// 		Sobel(mat, xgrad, CV_32F, 1, 0);
// 		Sobel(mat, ygrad, CV_32F, 0, 1);
// 		cv::Mat xygrad = xgrad.mul(xgrad) + ygrad.mul(ygrad);
// 		cv::sqrt(xygrad, xygrad);
// 		cv::Mat thmat;
// 		cv::threshold(xygrad, thmat, th, 1, cv::ThresholdTypes::THRESH_BINARY);
// 		cv::Mat resmat = xygrad.mul(thmat);
// 		cv::pow(resmat, 2, resmat);
// 		return static_cast<float>(mean(resmat)[0]);
//     }
// };

class Variance final : public ImageClarityEvaluationMethod {
public:
    virtual void crop(cv::Mat& mat, int x, int y) override {
        ImageClarityEvaluationMethod::crop(mat, x, y);
    }
    virtual void to_gray(cv::Mat& mat) override {
        ImageClarityEvaluationMethod::to_gray(mat);
    }
    virtual float clarity(cv::Mat& mat, int x, int y) override {
        // maix::log::info("crop start");
        // this->crop(mat, x, y);
        // maix::log::info("crop end");
        // this->to_gray(mat);

        using namespace cv;

        // maix::log::info("%s %d", __PRETTY_FUNCTION__, __LINE__);
        Mat mean_img, std_img;
		meanStdDev(mat, mean_img, std_img);
        // maix::log::info("%s %d", __PRETTY_FUNCTION__, __LINE__);
		return static_cast<float>(std_img.at<double>(0, 0));
    }
};

class EngeryOfGradient final : public ImageClarityEvaluationMethod {
public:
    virtual void crop(cv::Mat& mat, int x, int y) override {
        ImageClarityEvaluationMethod::crop(mat, x, y);
    }
    virtual void to_gray(cv::Mat& mat) override {
        ImageClarityEvaluationMethod::to_gray(mat);
    }
    virtual float clarity(cv::Mat& mat, int x, int y) override {
        // this->crop(mat, x, y);
        // this->to_gray(mat);

        using namespace cv;

        Mat k1 = (Mat_<char>(2, 1) << -1, 1);
		Mat k2 = (Mat_<char>(1, 2) << -1, 1);
		Mat e1, e2;
        // maix::log::info("%s %d", __PRETTY_FUNCTION__, __LINE__);
		filter2D(mat, e1, CV_32F, k1);
		filter2D(mat, e2, CV_32F, k2);
        // maix::log::info("%s %d", __PRETTY_FUNCTION__, __LINE__);
		Mat r = e1.mul(e1) + e2.mul(e2);
        // maix::log::info("%s %d", __PRETTY_FUNCTION__, __LINE__);
		return static_cast<float>(mean(r)[0]);
    }
};

class Brenner final : public ImageClarityEvaluationMethod {
public:
    virtual void crop(cv::Mat& mat, int x, int y) override {
        ImageClarityEvaluationMethod::crop(mat, x, y);
    }
    virtual void to_gray(cv::Mat& mat) override {
        ImageClarityEvaluationMethod::to_gray(mat);
    }
    virtual float clarity(cv::Mat& mat, int x, int y) override {
        // this->crop(mat, x, y);
        // this->to_gray(mat);

        using namespace cv;

        Mat kb = (Mat_<char>(3, 1) << -1, 0, 1);
		Mat bi;
		filter2D(mat, bi, CV_32F, kb);
		pow(bi, 2, bi);
		return static_cast<float>(mean(bi)[0]);
    }
};

class Laplace final : public ImageClarityEvaluationMethod {
public:
    virtual void crop(cv::Mat& mat, int x, int y) override {
        ImageClarityEvaluationMethod::crop(mat, x, y);
    }
    virtual void to_gray(cv::Mat& mat) override {
        ImageClarityEvaluationMethod::to_gray(mat);
    }
    virtual float clarity(cv::Mat& mat, int x, int y) override {
        // this->crop(mat, x, y);
        // this->to_gray(mat);

        using namespace cv;

        Mat kl = (Mat_<char>(3, 3) << -1, -1, -1, -1, 8, -1, -1, -1, -1);
		Mat li;
		filter2D(mat, li, CV_32F, kl);
		pow(li, 2, li);
		return static_cast<float>(mean(li)[0]);
    }
};

#endif // __APP_EX_HPP__