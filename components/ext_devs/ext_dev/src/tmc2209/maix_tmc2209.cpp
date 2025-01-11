#include "maix_tmc2209.hpp"
#include <iostream>
#include <fstream>
#include <deque>
#include <memory>
#include "TMC2209.h"

namespace maix::ext_dev::tmc2209 {

static constexpr uint16_t DEFAULT_SPEED_FACTOR = 50;
static constexpr float SPEED_EACH_FACTOR = 0.72;    // speed factor=1 ==> 0.72 fullsteps/s
static constexpr int TRY_NUM = 5;

/**
 * user input:  speed 10 mm/s, step_angle 1.8, screw_pitch: 5mm
 * 360 used time ==> screw_pitch/speed ==> 0.5s
 * 360 fullsteps ==> 360/step_angle ==> 200 fullsteps
 * speed ==> (360/step_angle)/(screw_pitch/speed) ==> 400 fullsteps/s
 * speed_factor ==> speed/SPEED_EACH_FACTOR ==> ((360/step_angle)/(screw_pitch/speed))/SPEED_EACH_FACTOR ==> 555
 */
static inline uint16_t calculate_speed_factor(float speed_mm_s, float step_angle, float screw_pitch)
{
    return static_cast<uint16_t>(((360.0f/step_angle)/(screw_pitch/speed_mm_s))/SPEED_EACH_FACTOR);
}

class FileHandler {
public:
    FileHandler() = delete;
    FileHandler(const FileHandler&) = delete;
    FileHandler& operator=(const FileHandler&) = delete;
    ~FileHandler() = delete;

    static bool file_exists(const std::string& filename) {
        std::ifstream file(filename);
        return file.good();
    }

    static bool is_path_valid(const std::string& path) {
        if (path.empty()) {
            maix::log::error("Path is empty.");
            return false;
        }
        // 检查路径中是否包含非法字符
        if (path.find_first_of("\\:*?\"<>|") != std::string::npos) {
            maix::log::error("Path contains invalid characters.");
            return false;
        }
        return true;
    }

    template<typename T>
    static bool write(const std::string& filename, const std::vector<T>& buffer)
    {
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs) {
            maix::log::error("Failed to open file for writing: %s", filename.c_str());
            return false;
        }

        ofs.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(T));
        if (!ofs.good())
            maix::log::error("Write operation failed.");
        return ofs.good();
    }

    template<typename T>
    static bool read(const std::string& filename, std::vector<T>& buffer) {
        if (!file_exists(filename)) {
            maix::log::error("File does not exist: %s", filename.c_str());
            return false;
        }
        std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
        if (!ifs) {
            maix::log::error("Failed to open file for reading: %s", filename.c_str());
            return false;
        }

        std::streamsize size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        buffer.resize(size / sizeof(T));
        ifs.read(reinterpret_cast<char*>(buffer.data()), size);
        if (!ifs.good())
            maix::log::error("Read operation failed.");
        return ifs.good();
    }

    static bool remove(const std::string& filename) {
        bool result = (std::remove(filename.c_str()) == 0);
        if (!result)
            maix::log::error("Remove operation failed.");
        return result;
    }

    static bool copy(const std::string& src_filename, const std::string& dest_filename) {
        if (!file_exists(src_filename)) {
            maix::log::error("Source file does not exist: %s", src_filename.c_str());
            return false;
        }

        std::ifstream src(src_filename, std::ios::binary);
        std::ofstream dest(dest_filename, std::ios::binary);

        if (!src || !dest) {
            maix::log::error("Failed to open file(s) for copying.");
            return false;
        }

        dest << src.rdbuf();
        if (!(src.good() && dest.good()))
            maix::log::error("Copy operation failed.");
        return src.good() && dest.good();
    }
};

static inline int32_t calc_velocity(uint16_t speed_factor, uint16_t micro_step, bool dir)
{
    int32_t vel = static_cast<int32_t>(speed_factor*micro_step);
    return dir ? vel : -vel;
}

#ifdef SLIDE_OLD_API
static inline bool check_micro_step(uint16_t micro_step)
{
    switch (micro_step) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
    case 32:
    case 64:
    case 128:
    case 256:
        return true; break;
    default: break;
    };
    return false;
}
#else
static inline bool check_micro_step(uint16_t micro_step)
{
    static const uint16_t valid_micro_steps[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
    for (size_t i = 0; i < sizeof(valid_micro_steps) / sizeof(valid_micro_steps[0]); ++i) {
        if (micro_step == valid_micro_steps[i]) {
            return true;
        }
    }
    return false;
}
#endif


static inline bool check_addr(uint8_t addr)
{
    return (addr>=4) ? false : true;
}

static uint16_t calculate_cnt_diff(uint16_t prev, uint16_t now, const int32_t dir)
{
    if (dir < 0) {
        std::swap(prev, now);
    }
    // maix::log::info("prev:%lu, now:%lu", prev, now);
    if (now == prev)
        return 1024;
    if (now > prev)
        return (now-prev);
    return (1024-prev+now);
}

template<typename T>
static T calculate_average(const std::deque<T>& deque) {
    if (deque.empty()) {
        return 0;
    }

    uint64_t sum = std::accumulate(deque.begin(), deque.end(), 0);
    return static_cast<T>(sum / deque.size());
}

template<typename T>
static T calculate_average(const std::vector<T>& numbers) {
    if (numbers.empty()) {
        return 0;
    }
    uint64_t sum = std::accumulate(numbers.begin(), numbers.end(), 0);
    return static_cast<T>(sum) / numbers.size();
}

template<typename T>
static inline T check_per(const T per)
{
    if (per < 0) return static_cast<T>(0);
    if (per > 100) return static_cast<T>(100);
    return per;
}

void slide_scan(const char* port, uint8_t addr, long baud, /* Uart init param */
                float step_angle, uint16_t micro_step, float round_mm, /* Motor init param */
                float speed_mm_s, bool dir, bool use_internal_sense_resistors, uint8_t run_current_per,
                uint8_t hold_current_per, const std::string conf_save_path,
                bool force_update  /* Driver init param */)
{
    if (speed_mm_s <= 0) {
        maix::log::info("speed <= 0, used default speed factor: %lu", DEFAULT_SPEED_FACTOR);
    }
    if (!check_addr(addr)) {
        maix::log::error("addr error. 0/1/2/3");
        return;
    }
    if (!check_micro_step(micro_step)) {
        maix::log::error("micro step err. 1/2/4/8/16/32/64/128/256");
        return;
    }
    run_current_per = check_per(run_current_per);
    hold_current_per = check_per(run_current_per);
    if (!FileHandler::is_path_valid(conf_save_path)) {
        return;
    }
    const uint16_t speed_factor = (speed_mm_s<=0) ? DEFAULT_SPEED_FACTOR : calculate_speed_factor(speed_mm_s, step_angle, round_mm);
    maix::log::info("speed_factor: %lu", speed_factor);
    const int32_t velcity = calc_velocity(speed_factor, micro_step, dir);
    TMC2209 stepper_driver;
    stepper_driver.setup(port, baud, static_cast<TMC2209::SerialAddress>(addr));
    maix::time::sleep_ms(100);
    if (use_internal_sense_resistors)
        stepper_driver.useInternalSenseResistors();
    else
        stepper_driver.useExternalSenseResistors();
    stepper_driver.setRunCurrent(run_current_per);
    stepper_driver.setHoldCurrent(hold_current_per);
    stepper_driver.enableCoolStep();
    stepper_driver.setMicrostepsPerStep(micro_step);
    stepper_driver.enable();
    // uint16_t step_cnt = stepper_driver.getMicrostepCounter();

    std::deque<uint16_t> queue;
    bool first_full = true;
    uint16_t runing_avg_sg = 0;
    for (int i = 0; i < TRY_NUM; ++i)
        stepper_driver.moveAtVelocity(velcity);
    uint64_t scan_start_time = maix::time::ticks_ms();

    while (!app::need_exit()) {
        maix::time::sleep_ms(5);
        uint16_t sg_result = stepper_driver.getStallGuardResult();
        // maix::log::info("sg:%lu", sg_result);
        queue.push_back(sg_result);
        if (queue.size() <= 20)
            continue;
        queue.pop_front();
        if (first_full) {
            first_full = false;
            runing_avg_sg = calculate_average(queue);
        }
    }
    for (int i = 0; i < TRY_NUM; ++i)
        stepper_driver.moveAtVelocity(0);
    uint16_t stop_avg_sg = calculate_average(queue);

    maix::log::info("Scan finish...");
    maix::log::info("Time: %llums, Run: %lu, Stop: %lu",
                    maix::time::ticks_ms()-scan_start_time, runing_avg_sg, stop_avg_sg);

    bool prev_conf_exists = false;
    uint16_t prev_runing_avg_sg = 0;
    uint16_t prev_stop_avg_sg = 0;
    while (FileHandler::file_exists(conf_save_path)) {
        std::vector<uint16_t> data;
        FileHandler::read(conf_save_path, data);
        if (data.size() != 2) {
            maix::log::error("Read results != 2");
            break;
        }
        prev_conf_exists = true;
        prev_runing_avg_sg = data[0];
        prev_stop_avg_sg = data[1];
        maix::log::info("prev_runing_avg_sg:%lu, prev_stop_avg_sg:%lu",
                            prev_runing_avg_sg, prev_stop_avg_sg);
        break;
    }

    if (force_update) {
        if (prev_conf_exists) {
            maix::log::info("force_update = true, ignore and clear prev data");
            FileHandler::remove(conf_save_path);
        }
        maix::log::info("Save to %s", conf_save_path);
        std::vector<uint16_t> new_date{runing_avg_sg, stop_avg_sg};
        FileHandler::write(conf_save_path, new_date);
        return;
    }

    if (prev_conf_exists) {
        std::vector<uint16_t> data{prev_runing_avg_sg, runing_avg_sg};
        runing_avg_sg = calculate_average(data);
        stop_avg_sg = std::max(stop_avg_sg, prev_stop_avg_sg);
        maix::log::info("force_update = false, prev_conf exists.");
        maix::log::info("new runing_avg_sg = %lu, new stop_avg_sg = %lu",
                        runing_avg_sg, stop_avg_sg);
        FileHandler::remove(conf_save_path);
        maix::log::info("Save to %s", conf_save_path);
        std::vector<uint16_t> new_date{runing_avg_sg, stop_avg_sg};
        FileHandler::write(conf_save_path, new_date);
        return;
    }

    maix::log::info("Save to %s", conf_save_path);
    std::vector<uint16_t> new_date{runing_avg_sg, stop_avg_sg};
    FileHandler::write(conf_save_path, new_date);
}

void slide_test(const char* port, uint8_t addr, long baud, /* Uart init param */
                float step_angle, uint16_t micro_step, float round_mm, /* Motor init param */
                float speed_mm_s, bool dir, bool use_internal_sense_resistors, uint8_t run_current_per,
                uint8_t hold_current_per, const std::string conf_save_path /* Driver init param */)
{
    if (speed_mm_s <= 0) {
        maix::log::info("speed <= 0, used default speed factor:%lu", DEFAULT_SPEED_FACTOR);
    }
    run_current_per = check_per(run_current_per);
    hold_current_per = check_per(run_current_per);
    if (!FileHandler::is_path_valid(conf_save_path)) {
        return;
    }
    const uint16_t speed_factor = (speed_mm_s<=0) ? DEFAULT_SPEED_FACTOR : calculate_speed_factor(speed_mm_s, step_angle, round_mm);
    maix::log::info("speed_factor: %lu", speed_factor);

    if (!FileHandler::file_exists(conf_save_path)) {
        maix::log::error("conf_file not exists!");
        return;
    }
    if (!check_micro_step(micro_step)) {
        maix::log::error("micro step err. 1/2/4/8/16/32/64/128/256");
        return;
    }
    if (!check_addr(addr)) {
        maix::log::error("addr error. 0/1/2/3");
        return;
    }
    std::vector<uint16_t> conf_data;
    FileHandler::read(conf_save_path, conf_data);
    if (conf_data.size() != 2) {
        maix::log::error("Read conf_data != 2");
        return;
    }
    maix::log::info("conf_running_avg_sg = %lu, conf_stop_avg_sg = %lu", conf_data[0], conf_data[1]);
    const int32_t velcity = calc_velocity(speed_factor, micro_step, dir);
    TMC2209 stepper_driver;
    stepper_driver.setup(port, baud, static_cast<TMC2209::SerialAddress>(addr));
    maix::time::sleep(1);
    if (use_internal_sense_resistors)
        stepper_driver.useInternalSenseResistors();
    else
        stepper_driver.useExternalSenseResistors();
    stepper_driver.setRunCurrent(run_current_per);
    stepper_driver.setHoldCurrent(hold_current_per);
    stepper_driver.enableCoolStep();
    stepper_driver.setMicrostepsPerStep(micro_step);
    stepper_driver.enable();

    std::deque<uint16_t> queue;
    for (int i = 0; i < TRY_NUM; ++i)
        stepper_driver.moveAtVelocity(velcity);
    while (!app::need_exit()) {
        maix::time::sleep_ms(1);
        uint16_t sg_result = stepper_driver.getStallGuardResult();
        queue.push_back(sg_result);
        if (queue.size() <= 5)
            continue;
        queue.pop_front();
        if (calculate_average(queue) < conf_data[1]) {
            app::set_exit_flag(true);
        }
    }
    for (int i = 0; i < TRY_NUM; ++i)
        stepper_driver.moveAtVelocity(0);
}


#ifdef SLIDE_OLD_API
void slide_scan(const char* port, uint8_t addr, long baud, bool dir,
            uint16_t micro_step, uint16_t speed_mm_s, bool use_internal_sense_resistors,
            uint8_t run_current_per, uint8_t hold_current_per,
            const char* conf_save_path, bool force_update)
{
    if (!check_micro_step(micro_step)) {
        maix::log::error("micro step err. 1/2/4/8/16/32/64/128/256");
        return;
    }
    if (!check_addr(addr)) {
        maix::log::error("addr error. 0/1/2/3");
        return;
    }
    const int32_t velcity = calc_velocity(DEFAULT_SPEED_FACTOR, micro_step, dir);
    TMC2209 stepper_driver;
    stepper_driver.setup(port, baud, static_cast<TMC2209::SerialAddress>(addr));
    maix::time::sleep(1);
    if (use_internal_sense_resistors)
        stepper_driver.useInternalSenseResistors();
    else
        stepper_driver.useExternalSenseResistors();
    stepper_driver.setRunCurrent(run_current_per);
    stepper_driver.setHoldCurrent(hold_current_per);
    stepper_driver.enableCoolStep();
    stepper_driver.setMicrostepsPerStep(micro_step);
    stepper_driver.enable();
    uint16_t step_cnt = stepper_driver.getMicrostepCounter();

    std::deque<uint16_t> queue;
    bool first_full = true;
    uint16_t runing_avg_sg = 0;
    stepper_driver.moveAtVelocity(velcity);
    uint64_t scan_start_time = maix::time::ticks_ms();

    while (!app::need_exit()) {
        maix::time::sleep_ms(5);
        uint16_t sg_result = stepper_driver.getStallGuardResult();
        maix::log::info("sg:%lu", sg_result);
        queue.push_back(sg_result);
        if (queue.size() <= 20)
            continue;
        queue.pop_front();
        if (first_full) {
            first_full = false;
            runing_avg_sg = calculate_average(queue);
        }
    }
    stepper_driver.moveAtVelocity(0);
    uint16_t stop_avg_sg = calculate_average(queue);

    maix::log::info("Scan finish...");
    maix::log::info("Time: %llums, Run: %lu, Stop: %lu",
                    maix::time::ticks_ms()-scan_start_time, runing_avg_sg, stop_avg_sg);

    bool prev_conf_exists = false;
    uint16_t prev_runing_avg_sg = 0;
    uint16_t prev_stop_avg_sg = 0;
    while (FileHandler::file_exists(conf_save_path)) {
        std::vector<uint16_t> data;
        FileHandler::read(conf_save_path, data);
        if (data.size() != 2) {
            maix::log::error("Read results != 2");
            break;
        }
        prev_conf_exists = true;
        prev_runing_avg_sg = data[0];
        prev_stop_avg_sg = data[1];
        maix::log::info("prev_runing_avg_sg:%lu, prev_stop_avg_sg:%lu",
                            prev_runing_avg_sg, prev_stop_avg_sg);
        break;
    }

    if (force_update) {
        if (prev_conf_exists) {
            maix::log::info("force_update = true, ignore and clear prev data");
            FileHandler::remove(conf_save_path);
        }
        maix::log::info("Save to %s", conf_save_path);
        std::vector<uint16_t> new_date{runing_avg_sg, stop_avg_sg};
        FileHandler::write(conf_save_path, new_date);
        return;
    }

    if (prev_conf_exists) {
        std::vector<uint16_t> data{prev_runing_avg_sg, runing_avg_sg};
        runing_avg_sg = calculate_average(data);
        stop_avg_sg = std::max(stop_avg_sg, prev_stop_avg_sg);
        maix::log::info("force_update = false, prev_conf exists.");
        maix::log::info("new runing_avg_sg = %lu, new stop_avg_sg = %lu",
                        runing_avg_sg, stop_avg_sg);
        FileHandler::remove(conf_save_path);
        maix::log::info("Save to %s", conf_save_path);
        std::vector<uint16_t> new_date{runing_avg_sg, stop_avg_sg};
        FileHandler::write(conf_save_path, new_date);
        return;
    }

    maix::log::info("Save to %s", conf_save_path);
    std::vector<uint16_t> new_date{runing_avg_sg, stop_avg_sg};
    FileHandler::write(conf_save_path, new_date);
}

void slide_test(const char* port, uint8_t addr, long baud, bool dir,
            uint16_t micro_step, bool use_internal_sense_resistors,
            uint8_t run_current_per, uint8_t hold_current_per,
            const char* conf_path)
{
    if (!FileHandler::file_exists(conf_path)) {
        maix::log::error("conf_file not exists!");
        return;
    }
    if (!check_micro_step(micro_step)) {
        maix::log::error("micro step err. 1/2/4/8/16/32/64/128/256");
        return;
    }
    if (!check_addr(addr)) {
        maix::log::error("addr error. 0/1/2/3");
        return;
    }
    std::vector<uint16_t> conf_data;
    FileHandler::read(conf_path, conf_data);
    if (conf_data.size() != 2) {
        maix::log::error("Read conf_data != 2");
        return;
    }
    maix::log::info("conf_running_avg_sg = %lu, conf_stop_avg_sg = %lu", conf_data[0], conf_data[1]);
    const int32_t velcity = calc_velocity(DEFAULT_SPEED_FACTOR, micro_step, dir);
    TMC2209 stepper_driver;
    stepper_driver.setup(port, baud, static_cast<TMC2209::SerialAddress>(addr));
    maix::time::sleep(1);
    if (use_internal_sense_resistors)
        stepper_driver.useInternalSenseResistors();
    else
        stepper_driver.useExternalSenseResistors();
    stepper_driver.setRunCurrent(run_current_per);
    stepper_driver.setHoldCurrent(hold_current_per);
    stepper_driver.enableCoolStep();
    stepper_driver.setMicrostepsPerStep(micro_step);
    stepper_driver.enable();

    std::deque<uint16_t> queue;
    stepper_driver.moveAtVelocity(velcity);
    while (!app::need_exit()) {
        maix::time::sleep_ms(1);
        uint16_t sg_result = stepper_driver.getStallGuardResult();
        queue.push_back(sg_result);
        if (queue.size() <= 5)
            continue;
        queue.pop_front();
        if (calculate_average(queue) < conf_data[1]) {
            app::set_exit_flag(true);
        }
    }
    stepper_driver.moveAtVelocity(0);
}
#endif


Slide::Slide(const char* port, uint8_t addr, long baud, /* Uart init param */
                float step_angle, uint16_t micro_step, float round_mm,   /* Motor init param */
                float speed_mm_s, bool use_internal_sense_resistors, uint8_t run_current_per,
                uint8_t hold_current_per, std::string cfg_file_path /* Driver init param */)
    : tmc2209(std::unique_ptr<TMC2209>(new TMC2209())), step_angle(step_angle), micro_step(micro_step), round_mm(round_mm)
{
    if (!check_micro_step(micro_step)) {
        maix::log::error("micro step err. 1/2/4/8/16/32/64/128/256");
        return;
    }
    if (!check_addr(addr)) {
        maix::log::error("addr error. 0/1/2/3");
        throw std::runtime_error("Addr error");
    }
    if (speed_mm_s <= 0) {
        this->speed_factor_ = DEFAULT_SPEED_FACTOR;
        maix::log::info("speed <= 0, used default speed factor: %lu if move() not pass speed_mm_s", DEFAULT_SPEED_FACTOR);
    } else
        this->speed_factor_ = calculate_speed_factor(speed_mm_s, step_angle, round_mm);
    this->tmc2209->setup(port, baud, static_cast<TMC2209::SerialAddress>(addr));
    this->stop_default_per(60);
    maix::time::sleep_ms(50);
    this->tmc2209->setMicrostepsPerStep(micro_step);
    this->micro_step_distance = static_cast<float>((round_mm * step_angle) / (360.0 * 256));
    this->load_conf(cfg_file_path);
    this->use_internal_sense_resistors(use_internal_sense_resistors);
    this->run_current_per(run_current_per);
    this->hold_current_per(hold_current_per);
    this->tmc2209->setStandstillMode(TMC2209::StandstillMode::NORMAL);
    this->tmc2209->enableAutomaticCurrentScaling();
    this->tmc2209->enableAutomaticGradientAdaptation();
    this->tmc2209->enableCoolStep();
    this->tmc2209->setMicrostepsPerStep(micro_step);
}

Slide::~Slide() {}

void Slide::load_conf(std::string path)
{
    if (path.empty()) {
        return;
    }
    if (!FileHandler::file_exists(path)) {
        maix::log::error("conf_file not exists!");
        return;
    }
    std::vector<uint16_t> conf_data;
    FileHandler::read(path, conf_data);
    if (conf_data.size() != 2) {
        maix::log::error("Read conf_data != 2");
        return;
    }
    maix::log::info("conf_running_avg_sg = %lu, conf_stop_avg_sg = %lu", conf_data[0], conf_data[1]);
    this->running_avg_sg = conf_data[0];
    this->stop_avg_sg = conf_data[1];
    this->used_conf_init = true;
}

void Slide::move(float oft, int speed_mm_s, bool check)
{
    uint16_t speed_factor = (speed_mm_s<=0) ? \
                                this->speed_factor_ : \
                                calculate_speed_factor(static_cast<uint16_t>(speed_mm_s),
                                    this->step_angle, this->round_mm);
    uint64_t required_micro_steps = static_cast<uint64_t>(std::abs(oft) / this->micro_step_distance);
    auto required_micro_steps_bak = required_micro_steps;
    required_micro_steps = this->error_handler.target(required_micro_steps);
    if (required_micro_steps >= required_micro_steps_bak * 2) {
        required_micro_steps = required_micro_steps_bak;
        this->error_handler.clear(required_micro_steps);
    }
    const int32_t velocity = calc_velocity(speed_factor, this->micro_step, (oft>=0));

    this->tmc2209->enable();
    this->run_current_per(this->run_current_per_);
    this->hold_current_per(this->hold_current_per_);
    uint16_t step_cnt = this->tmc2209->getMicrostepCounter();
    uint64_t total_mscnt2 = 0;
    uint16_t prev_mscnt2 = step_cnt;
    int uninit_avg_sg = -1;
    uint16_t uninit_stop_sg = 0;
    std::deque<uint16_t> queue;

    // for (int i = 0; i < TRY_NUM; ++i)
    this->tmc2209->moveAtVelocity(velocity);

    while (true && !maix::app::need_exit()) {
        this->tmc2209->moveAtVelocity(velocity);
        maix::time::sleep_ms(1);
        step_cnt = this->tmc2209->getMicrostepCounter();
        total_mscnt2 += calculate_cnt_diff(prev_mscnt2, step_cnt, velocity);
        prev_mscnt2 = step_cnt;
        // maix::log::info("curr:%llu, target:%llu", total_mscnt2, required_micro_steps);
        if (total_mscnt2 >= required_micro_steps) {
            this->error_handler.save_error(total_mscnt2);
            break;
        }
        if (!check) continue;
        uint16_t sg_result = this->tmc2209->getStallGuardResult();
        queue.push_back(sg_result);
        if (!this->used_conf_init) {    /** No conf */
            if (queue.size() <= 10) continue;
            queue.pop_front();
            if (uninit_avg_sg == -1) {
                uninit_avg_sg = static_cast<int>(calculate_average(queue));
                uninit_stop_sg = static_cast<uint16_t>(
                    static_cast<uint32_t>(uninit_avg_sg)*this->stop_default_per_/100);
            } else if (calculate_average(queue) < uninit_stop_sg) {
                this->error_handler.save_error(required_micro_steps);
                break;
            }
        } else {    /** Conf */
            if (queue.size() <= 5) continue;
            queue.pop_front();
            if (calculate_average(queue) < this->stop_avg_sg) {
                this->error_handler.save_error(required_micro_steps);
                break;
            }
        }
    }
    for (int i = 0; i < TRY_NUM; ++i) {
        this->tmc2209->moveAtVelocity(0);
        this->tmc2209->disable();
        this->tmc2209->enable();
        this->hold_current_per(this->hold_current_per_);
    }
}

void Slide::reset(bool dir, int speed_mm_s)
{
    uint16_t speed_factor = (speed_mm_s<=0) ? \
                                this->speed_factor_ : \
                                calculate_speed_factor(static_cast<uint16_t>(speed_mm_s),
                                    this->step_angle, this->round_mm);
    const int32_t velocity = calc_velocity(speed_factor, this->micro_step, dir);
    this->tmc2209->enable();

    int uninit_avg_sg = -1;
    uint16_t uninit_stop_sg = 0;
    std::deque<uint16_t> queue;

    for (int i = 0; i < TRY_NUM; ++i)
        this->tmc2209->moveAtVelocity(velocity);
    while (true && !maix::app::need_exit()) {
        maix::time::sleep_ms(1);
        uint16_t sg_result = this->tmc2209->getStallGuardResult();
        queue.push_back(sg_result);
        if (!this->used_conf_init) {
            if (queue.size() <= 10) continue;
            queue.pop_front();
            if (uninit_avg_sg == -1) {
                uninit_avg_sg = static_cast<int>(calculate_average(queue));
                uninit_stop_sg = static_cast<uint16_t>(
                    static_cast<uint32_t>(uninit_avg_sg)*this->stop_default_per_/100);
            } else if (calculate_average(queue) < uninit_stop_sg) {
                break;
            }
        } else {
            if (queue.size() <= 5) continue;
            queue.pop_front();
            if (calculate_average(queue) < this->stop_avg_sg) break;
        }
    }
    for (int i = 0; i < TRY_NUM; ++i) {
        this->tmc2209->moveAtVelocity(0);
        // this->tmc2209->disable();
        this->hold_current_per(this->hold_current_per_);
    }
}

int Slide::stop_default_per(int per)
{
    if (per < 0)
        return static_cast<int>(this->stop_default_per_);
    per = check_per(per);
    this->stop_default_per_ = per;
    return per;
}

int Slide::run_current_per(int per)
{
    if (per < 0)
        return static_cast<int>(this->run_current_per_);
    per = check_per(per);
    this->run_current_per_ = static_cast<uint8_t>(per);
    this->tmc2209->setRunCurrent(this->run_current_per_);
    return per;
}

int Slide::hold_current_per(int per)
{
    if (per < 0)
        return static_cast<int>(this->hold_current_per_);
    per = check_per(per);
    this->hold_current_per_ = static_cast<uint8_t>(per);
    this->tmc2209->setHoldCurrent(this->hold_current_per_);
    return per;
}

void Slide::use_internal_sense_resistors(bool b)
{
    if (b)
        this->tmc2209->useInternalSenseResistors();
    else
        this->tmc2209->useExternalSenseResistors();
}


ScrewSlide::ScrewSlide(const char* port, uint8_t addr, long baud, /* Uart init param */
                float step_angle, uint16_t micro_step, float screw_pitch,   /* Motor init param */
                float speed_mm_s, bool use_internal_sense_resistors, uint8_t run_current_per,
                uint8_t hold_current_per)
            : tmc2209(std::unique_ptr<TMC2209>(new TMC2209())), step_angle(step_angle), micro_step(micro_step), screw_pitch(screw_pitch)
{
    if (!check_micro_step(micro_step)) {
        maix::log::error("micro step err. 1/2/4/8/16/32/64/128/256");
        throw std::runtime_error("micro_step error");
    }
    if (!check_addr(addr)) {
        maix::log::error("addr error. 0/1/2/3");
        throw std::runtime_error("Addr error");
    }
    if (speed_mm_s <= 0) {
        this->speed_factor_ = DEFAULT_SPEED_FACTOR;
        maix::log::info("speed <= 0, used default speed factor: %lu if move() not pass speed_mm_s", DEFAULT_SPEED_FACTOR);
    } else
        this->speed_factor_ = calculate_speed_factor(speed_mm_s, step_angle, screw_pitch);
    this->tmc2209->setup(port, baud, static_cast<TMC2209::SerialAddress>(addr));
    maix::time::sleep_ms(50);
    this->tmc2209->setMicrostepsPerStep(micro_step);
    this->micro_step_distance = static_cast<float>((screw_pitch * step_angle) / (360.0 * 256));
    this->use_internal_sense_resistors(use_internal_sense_resistors);
    this->run_current_per(run_current_per);
    this->hold_current_per(hold_current_per);
    this->tmc2209->setStandstillMode(TMC2209::StandstillMode::NORMAL);
    this->tmc2209->enableAutomaticCurrentScaling();
    this->tmc2209->enableAutomaticGradientAdaptation();
    this->tmc2209->enableCoolStep();
    this->tmc2209->setMicrostepsPerStep(micro_step);
}

ScrewSlide::~ScrewSlide() {}

void ScrewSlide::move(float oft, int speed_mm_s, std::function<bool(float)> callback)
{
    uint16_t speed_factor = (speed_mm_s<=0) ? \
                                this->speed_factor_ : \
                                calculate_speed_factor(static_cast<uint16_t>(speed_mm_s),
                                    this->step_angle, this->screw_pitch);
    // maix::log::info("speed factor:%u", speed_factor);
    // maix::log::info("std::abs(oft): %0.3f", std::abs(oft));
    // maix::log::info("this->micro_step_distance: %0.8f", this->micro_step_distance);
    // maix::log::info("std::abs(oft) / this->micro_step_distance: %0.3f", std::abs(oft) / this->micro_step_distance);
    uint64_t required_micro_steps = static_cast<uint64_t>(std::abs(oft) / this->micro_step_distance);
    auto required_micro_steps_bak = required_micro_steps;
    // maix::log::info("required_micro_steps: %llu", required_micro_steps);
    // printf("target:%llu, ", required_micro_steps);
    required_micro_steps = this->error_handler.target(required_micro_steps);
    if (required_micro_steps >= required_micro_steps_bak) {
        required_micro_steps = required_micro_steps_bak;
        this->error_handler.clear(required_micro_steps);
    }
    // maix::log::info("after target: %llu", required_micro_steps);
    const int32_t velocity = calc_velocity(speed_factor, this->micro_step, (oft>=0));

    this->tmc2209->enable();
    this->run_current_per(this->run_current_per_);
    this->hold_current_per(this->hold_current_per_);
    uint16_t step_cnt = this->tmc2209->getMicrostepCounter();
    uint64_t total_mscnt2 = 0;
    uint16_t prev_mscnt2 = step_cnt;

    // maix::log::info("Start...total_mscnt2:%llu", total_mscnt2);

    for (int i = 0; i < TRY_NUM; ++i)
        this->tmc2209->moveAtVelocity(velocity);

    while (true && !maix::app::need_exit()) {
        // this->tmc2209->moveAtVel ocity(velocity);
        // maix::time::sleep_ms(1);
        step_cnt = this->tmc2209->getMicrostepCounter();
        // maix::log::info("step_cnt:%llu", step_cnt);
        total_mscnt2 += calculate_cnt_diff(prev_mscnt2, step_cnt, velocity);
        prev_mscnt2 = step_cnt;
        // if ((oft>=0))
        //     maix::log::info("curr:%llu, target:%llu++++++++++[%ld]", total_mscnt2, required_micro_steps, velocity);
        // else
        //     maix::log::info("curr:%llu, target:%llu----------[%ld]", total_mscnt2, required_micro_steps, velocity);
        if (total_mscnt2 >= required_micro_steps) break;
        if (callback == nullptr) continue;
        if (callback(static_cast<float>(static_cast<double>(total_mscnt2)/required_micro_steps*100.0))) break;
    }
    // maix::log::info("left loop");
    for (int i = 0; i < TRY_NUM; ++i) {
        // maix::log::info("Stop");
        this->tmc2209->moveAtVelocity(0);
        // this->tmc2209->dissable();
        this->hold_current_per(this->hold_current_per_);
    }
    [[maybe_unused]]uint64_t err = (maix::app::need_exit()) ? 0 : this->error_handler.save_error(total_mscnt2);
    // maix::log::info("err:%llu\n\n", err);
}

void ScrewSlide::reset(std::function<bool(void)> callback, bool dir, int speed_mm_s)
{
    if (callback == nullptr) {
        maix::log::error("callback is NONE");
        return;
    }
    uint16_t speed_factor = (speed_mm_s<=0) ? \
                                this->speed_factor_ : \
                                calculate_speed_factor(static_cast<uint16_t>(speed_mm_s),
                                    this->step_angle, this->screw_pitch);
    const int32_t velocity = calc_velocity(speed_factor, this->micro_step, dir);
    this->tmc2209->enable();

    for (int i = 0; i < TRY_NUM; ++i)
        this->tmc2209->moveAtVelocity(velocity);
    while (true && !maix::app::need_exit()) {
        maix::time::sleep_ms(1);
        if (callback()) break;
    }

    for (int i = 0; i < TRY_NUM; ++i) {
        this->tmc2209->moveAtVelocity(0);
        // this->tmc2209->disable();
        this->hold_current_per(this->hold_current_per_);
    }
}

int ScrewSlide::run_current_per(int per)
{
    if (per < 0)
        return static_cast<int>(this->run_current_per_);
    per = check_per(per);
    this->run_current_per_ = static_cast<uint8_t>(per);
    this->tmc2209->setRunCurrent(this->run_current_per_);
    return per;
}

int ScrewSlide::hold_current_per(int per)
{
    if (per < 0)
        return static_cast<int>(this->hold_current_per_);
    per = check_per(per);
    this->hold_current_per_ = static_cast<uint8_t>(per);
    this->tmc2209->setHoldCurrent(this->hold_current_per_);
    return per;
}

void ScrewSlide::use_internal_sense_resistors(bool b)
{
    if (b)
        this->tmc2209->useInternalSenseResistors();
    else
        this->tmc2209->useExternalSenseResistors();
}

}

