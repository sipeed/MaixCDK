#include <memory>
#include <string>
#include <cmath>
#include "maix_basic.hpp"
#include <functional>

class TMC2209;

namespace maix::ext_dev::tmc2209 {

template<typename T>
class SlideErrorHandler {
public:
    SlideErrorHandler() : prev_error(T{}), target_(0), prev_err_flag(true) {}
    ~SlideErrorHandler() {}

    T target(T now) noexcept {
        this->target_ = (this->prev_err_flag) ? (now + this->prev_error) : (now - this->prev_error);
        return this->target_;
    }

    T save_error(T result) noexcept {
        this->prev_err_flag = (result <= this->target_);
        this->prev_error = (this->prev_err_flag) ? (this->target_ - result) : (result - this->target_);
        return this->prev_error;
    }

    void clear(T now) noexcept {
        this->prev_error = T{};
        this->target_ = now;
    }
public:
    T prev_error;
    T target_;
    bool prev_err_flag;
};

// #define SLIDE_OLD_API

#ifdef SLIDE_OLD_API
void slide_scan(const char* port, uint8_t addr, long baud=115200, bool dir=true,
            uint16_t micro_step=256, uint16_t speed_mm_s, bool use_internal_sense_resistors=true,
            uint8_t run_current_per=50, uint8_t hold_current_per=50,
            const char* conf_save_path="./slide_conf.bin", bool force_update=true);
#endif // SLIDE_OLD_API

/**
 * @brief Scan and initialize the slide with the given parameters
 *
 * @param port UART port, string type.
 * @param addr TMC2209 UART address, range 0x00~0x03, integer type.
 * @param baud UART baud rate, integer type.
 * @param step_angle Motor step angle, float type.
 * @param micro_step Motor micro step, options: 1/2/4/8/16/32/64/128/256, integer type.
 * @param round_mm Round distance in mm, float type.
 * @param speed_mm_s Speed of the slide in mm/s, float type.
 * @param dir Direction of movement, boolean type. Default is true.
 * @param use_internal_sense_resistors Enable internal sense resistors if true, disable if false, boolean type. Default is true.
 * @param run_current_per Motor run current percentage, range 0~100(%), integer type. Default is 100%.
 * @param hold_current_per Motor hold current percentage, range 0~100(%), integer type. Default is 100%.
 * @param conf_save_path Configuration save path, string type. Default is "./slide_conf.bin".
 * @param force_update Force update the configuration if true, boolean type. Default is true.
 *
 * @maixpy maix.ext_dev.tmc2209.slide_scan
 */
void slide_scan(const char* port, uint8_t addr, long baud, /* Uart init param */
                float step_angle, uint16_t micro_step, float round_mm, /* Motor init param */
                float speed_mm_s, bool dir=true, bool use_internal_sense_resistors=true, uint8_t run_current_per=100,
                uint8_t hold_current_per=100, const std::string conf_save_path="./slide_conf.bin",
                bool force_update=true  /* Driver init param */);


#ifdef SLIDE_OLD_API
void slide_test(const char* port, uint8_t addr, long baud=115200, bool dir=true,
            uint16_t micro_step=256, bool use_internal_sense_resistors=true,
            uint8_t run_current_per=50, uint8_t hold_current_per=50,
            const char* conf_path="./slide_conf.bin");
#endif // SLIDE_OLD_API


/**
 * @brief Test the slide with the given parameters
 *
 * This function tests the slide by moving it in the specified direction until a stall condition is detected, as defined in the configuration file.
 *
 * @param port UART port, string type.
 * @param addr TMC2209 UART address, range 0x00~0x03, integer type.
 * @param baud UART baud rate, integer type.
 * @param step_angle Motor step angle, float type.
 * @param micro_step Motor micro step, options: 1/2/4/8/16/32/64/128/256, integer type.
 * @param round_mm Round distance in mm, float type.
 * @param speed_mm_s Speed of the slide in mm/s, float type.
 * @param dir Direction of movement, boolean type. Default is true.
 * @param use_internal_sense_resistors Enable internal sense resistors if true, disable if false, boolean type. Default is true.
 * @param run_current_per Motor run current percentage, range 0~100(%), integer type. Default is 100%.
 * @param hold_current_per Motor hold current percentage, range 0~100(%), integer type. Default is 100%.
 * @param conf_save_path Configuration save path, string type. Default is "./slide_conf.bin".
 *
 * @maixpy maix.ext_dev.tmc2209.slide_test
 */
void slide_test(const char* port, uint8_t addr, long baud, /* Uart init param */
                float step_angle, uint16_t micro_step, float round_mm, /* Motor init param */
                float speed_mm_s, bool dir=true, bool use_internal_sense_resistors=true, uint8_t run_current_per=100,
                uint8_t hold_current_per=100, const std::string conf_save_path="./slide_conf.bin"/* Driver init param */);

/**
 * Slide Class
 * @maixpy maix.ext_dev.tmc2209.Slide
 */
class Slide {
public:
    /**
     * @brief Constructor for Slide
     *
     * Initializes the Slide object with the specified parameters.
     *
     * @param port UART port, string type.
     * @param addr TMC2209 UART address, range 0x00~0x03, integer type.
     * @param baud UART baud rate, integer type.
     * @param step_angle Motor step angle, float type.
     * @param micro_step Motor micro step, options: 1/2/4/8/16/32/64/128/256, integer type.
     * @param round_mm Round distance in mm, float type.
     * @param speed_mm_s Speed of the slide in mm/s, float type. Default is -1, indicating the use of a default speed factor.
     * @param use_internal_sense_resistors Enable internal sense resistors if TRUE, disable if FALSE, boolean type. Default is TRUE.
     * @param run_current_per Motor run current percentage, range 0~100(%), integer type. Default is 100%.
     * @param hold_current_per Motor hold current percentage, range 0~100(%), integer type. Default is 100%.
     * @param cfg_file_path Configuration file path, string type. Default is an empty string, indicating no configuration file.
     *
     * @maixpy maix.ext_dev.tmc2209.Slide.__init__
     */
    Slide(const char* port, uint8_t addr, long baud, /* Uart init param */
            float step_angle, uint16_t micro_step, float round_mm,   /* Motor init param */
            float speed_mm_s=-1, bool use_internal_sense_resistors=true, uint8_t run_current_per=100,
            uint8_t hold_current_per=100, std::string cfg_file_path="" /* Driver init param */);

    Slide(const Slide&)               = delete;
    Slide& operator=(const Slide&)    = delete;
    Slide(Slide&&)                    = default;
    Slide& operator=(Slide&&)         = default;

    ~Slide();

    /**
     * @brief Load configuration from a file
     *
     * Loads the configuration settings for the slide from the specified file path.
     *
     * @param path Path to the configuration file, string type.
     *
     * @maixpy maix.ext_dev.tmc2209.Slide.load_conf
     */
    void load_conf(std::string path);

    /**
     * @brief Move the slide by a specified length
     *
     * Moves the slide by the specified length at the given speed. Optionally checks for stall conditions.
     *
     * @param oft Length to move, float type.
     * @param speed_mm_s Speed in mm/s. Default is -1, indicating the use of the default speed set during initialization.
     * @param check Enable movement check if true, boolean type. Default is true.
     *
     * @maixpy maix.ext_dev.tmc2209.Slide.move
     */
    void move(float oft, int speed_mm_s=-1, bool check=true);

    /**
     * @brief Reset the slide position
     *
     * Resets the slide position in the specified direction at the given speed.
     *
     * @param dir Direction of reset, boolean type. Default is false.
     * @param speed_mm_s Speed in mm/s. Default is -1, indicating the use of the speed set during initialization.
     *
     * @maixpy maix.ext_dev.tmc2209.Slide.reset
     */
    void reset(bool dir=false, int speed_mm_s=-1);

    /**
     * @brief Get or set the stop default percentage
     *
     * Retrieves or sets the stop default percentage. If the parameter is -1, it returns the current setting.
     *
     * @param per Stop default percentage, range 0~100(%), integer type. Default is -1, indicating no change.
     * @return int Current stop default percentage if per is -1, otherwise the new set percentage.
     *
     * @maixpy maix.ext_dev.tmc2209.Slide.stop_default_per
     */
    int stop_default_per(int per=-1);

    /**
     * @brief Get or set the run current percentage
     *
     * Retrieves or sets the run current percentage. If the parameter is -1, it returns the current setting.
     *
     * @param per Run current percentage, range 0~100(%), integer type. Default is -1, indicating no change.
     * @return int Current run current percentage if per is -1, otherwise the new set percentage.
     *
     * @maixpy maix.ext_dev.tmc2209.Slide.run_current_per
     */
    int run_current_per(int per=-1);

    /**
     * @brief Get or set the hold current percentage
     *
     * Retrieves or sets the hold current percentage. If the parameter is -1, it returns the current setting.
     *
     * @param per Hold current percentage, range 0~100(%), integer type. Default is -1, indicating no change.
     * @return int Current hold current percentage if per is -1, otherwise the new set percentage.
     *
     * @maixpy maix.ext_dev.tmc2209.Slide.hold_current_per
     */
    int hold_current_per(int per=-1);

    /**
     * @brief Enable or disable internal sense resistors
     *
     * Enables or disables the internal sense resistors based on the provided boolean value.
     *
     * @param b Boolean value to enable (true) or disable (false) internal sense resistors. Default is true.
     *
     * @maixpy maix.ext_dev.tmc2209.Slide.use_internal_sense_resistors
     */
    void use_internal_sense_resistors(bool b=true);

public:
    uint8_t stop_default_per_;

private:
    std::unique_ptr<TMC2209> tmc2209{nullptr};
    float step_angle;
    uint16_t micro_step;
    float round_mm;
    float micro_step_distance;
    uint8_t run_current_per_;
    uint8_t hold_current_per_;
    uint16_t running_avg_sg;
    uint16_t stop_avg_sg;
    bool used_conf_init{false};
    uint16_t speed_factor_;
    SlideErrorHandler<uint64_t> error_handler;
};



/**
 * ScrewSlide Class
 * @maixpy maix.ext_dev.tmc2209.ScrewSlide
 */
class ScrewSlide {
public:
    /**
     * @brief Constructor for ScrewSlide
     *
     * @param port UART port, string type.
     * @param addr TMC2209 UART address, range 0x00~0x03, integer type.
     * @param baud UART baud rate, integer type.
     * @param step_angle Motor step angle, float type.
     * @param micro_step Motor micro step, options: 1/2/4/8/16/32/64/128/256, integer type.
     * @param screw_pitch Screw pitch of the slide, integer type.
     * @param speed_mm_s Speed of the slide in mm/s, 10 means 10mm/s, float type.
     * Default is -1, indicating the use of a default speed factor.
     * @param use_internal_sense_resistors Enable internal sense resistors if TRUE,
     * disable if FALSE, boolean type. Default is TRUE.
     * @param run_current_per Motor run current percentage, range 0~100(%), integer type. Default is 100%.
     * @param hold_current_per Motor hold current percentage, range 0~100(%), integer type. Default is 100%.
     *
     * @maixpy maix.ext_dev.tmc2209.ScrewSlide.__init__
     */
    ScrewSlide(const char* port, uint8_t addr, long baud, /* Uart init param */
                float step_angle, uint16_t micro_step, float screw_pitch,   /* Motor init param */
                float speed_mm_s=-1, bool use_internal_sense_resistors=true, uint8_t run_current_per=100,
                uint8_t hold_current_per=100);

    ScrewSlide(const ScrewSlide&)               = delete;
    ScrewSlide& operator=(const ScrewSlide&)    = delete;
    ScrewSlide(ScrewSlide&&)                    = default;
    ScrewSlide& operator=(ScrewSlide&&)         = default;

    ~ScrewSlide();

    /**
     * @brief Move the slide by a specified length
     *
     * @param oft Length to move, 10 means 10mm, float type.
     * Positive values move the slide in the positive direction, negative values move it in the opposite direction.
     * @param speed_mm_s Speed in mm/s. Default is -1, indicating the use of the default speed set during initialization.
     * @param callback Callback function to be called during movement.
     * The callback function receives the current progress percentage (0~100%) of the movement.
     * If the callback returns true, the move operation will be terminated immediately. Default is nullptr.
     *
     * @maixpy maix.ext_dev.tmc2209.ScrewSlide.move
     */
    void move(float oft, int speed_mm_s=-1, std::function<bool(float)> callback=nullptr);

    /**
     * @brief Reset the slide position
     *
     * @param callback Callback function to be called during the reset loop.
     * The reset operation will only terminate if the callback returns true.
     * @param dir Direction of reset. Default is false.
     * @param speed_mm_s Speed in mm/s. Default is -1, indicating the use of the speed set during initialization.
     *
     * @maixpy maix.ext_dev.tmc2209.ScrewSlide.reset
     */
    void reset(std::function<bool(void)> callback, bool dir=false, int speed_mm_s=-1);


    /**
     * @brief Get or set the run current percentage
     *
     * @param per Run current percentage, range 0~100(%).
     * Default is -1, indicating no change and returning the current run current percentage.
     * @return int Current run current percentage if per is -1, otherwise the new set percentage.
     *
     * @maixpy maix.ext_dev.tmc2209.ScrewSlide.run_current_per
     */
    int run_current_per(int per=-1);

    /**
     * @brief Get or set the hold current percentage
     *
     * @param per Hold current percentage, range 0~100(%). Default is -1, indicating no change and returning the current hold current percentage.
     * @return int Current hold current percentage if per is -1, otherwise the new set percentage.
     *
     * @maixpy maix.ext_dev.tmc2209.ScrewSlide.hold_current_per
     */
    int hold_current_per(int per=-1);

    /**
     * @brief Enable or disable internal sense resistors
     *
     * @param b Boolean value to enable (true) or disable (false) internal sense resistors. Default is true.
     *
     * @maixpy maix.ext_dev.tmc2209.ScrewSlide.use_internal_sense_resistors
     */
    void use_internal_sense_resistors(bool b=true);


private:
    std::unique_ptr<TMC2209> tmc2209{nullptr};
    float step_angle;
    uint16_t micro_step;
    float screw_pitch;
    float micro_step_distance;
    uint8_t run_current_per_;
    uint8_t hold_current_per_;
    uint16_t speed_factor_;
    SlideErrorHandler<uint64_t> error_handler;
};

}