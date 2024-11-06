#include <thread>
#include "maix_basic.hpp"
#include "main.h"
#include "maix_tmc2209.hpp"
#include "maix_display.hpp"
#include "maix_camera.hpp"
#include "maix_pinmap.hpp"

using namespace maix;

constexpr int MODE_SHOW_PICTURE = 0; // Mode constant for showing pictures
constexpr int MODE_SHOW_VIDEO = 1;    // Mode constant for showing videos

/* Change mode according to your needs */
constexpr auto MODE = MODE_SHOW_PICTURE; // Set the mode (picture or video)

int _main(int argc, char* argv[])
{
    // Compile-time conditional checks based on the selected mode
    if constexpr (MODE == MODE_SHOW_PICTURE) {
        maix::log::info("Mode: show picture");      // Log message for picture mode
    } else if constexpr (MODE == MODE_SHOW_VIDEO) {
        maix::log::info("Mode: show video");        // Log message for video mode
    } else {
        maix::log::error("Error Mode %d", MODE);    // Log error for unknown mode
        return -1;                                  // Exit with error
    }

    /* Initialize camera and display */
    camera::Camera cam(512, 320); // Create camera object with resolution 512x320
    display::Display disp;         // Create display object

    /* UART configuration: UART1 and 115200 baudrate */
    const auto port = "/dev/ttyS1"s; // UART port string
    constexpr long uart_baudrate = 115200;  // UART baudrate constant

    /* Motor X UART address and parameters */
    constexpr uint8_t uart_addr_x = 0x00;       // UART address for motor X
    constexpr uint8_t step_angle_x = 18;        // Step angle for motor X (in degrees)
    constexpr uint16_t micro_step_x = 256;      // Microstepping factor for motor X
    constexpr float screw_pitch_x = 3.0f;       // Screw pitch for motor X (in mm)
    constexpr float speed_x = 1.0f;             // Speed for motor X (in mm/s)
    constexpr bool dir_x = true;                // Direction for motor X (true for one direction)
    constexpr int32_t start_x = 0;              // Start position for motor X
    constexpr int32_t stop_x = 20;              // Stop position for motor X
    constexpr int32_t step_x = 3;               // Step size for motor X (in mm)

    /* Motor Y UART address and parameters */
    constexpr uint8_t uart_addr_y = 0x03;       // UART address for motor Y
    constexpr uint8_t step_angle_y = 18;        // Step angle for motor Y (in degrees)
    constexpr uint16_t micro_step_y = 256;      // Microstepping factor for motor Y
    constexpr float screw_pitch_y = 3.0f;       // Screw pitch for motor Y (in mm)
    constexpr float speed_y = 1.0f;             // Speed for motor Y (in mm/s)
    constexpr bool dir_y = false;               // Direction for motor Y (false for opposite direction)
    constexpr int32_t start_y = 0;              // Start position for motor Y
    constexpr int32_t stop_y = 15;              // Stop position for motor Y
    constexpr int32_t step_y = 3;               // Step size for motor Y (in mm)

    /* Enable UART1 */
    auto set_uart_pin = [](const std::string& pin, const std::string& function) {
        auto ret = peripheral::pinmap::set_pin_function(pin.c_str(), function.c_str());
        if (ret != err::Err::ERR_NONE) {
            maix::log::error("pinmap error");
            return false;
        }
        return true;
    };

    if (port == "/dev/ttyS1") {
        if (!set_uart_pin("A19", "UART1_TX") || !set_uart_pin("A18", "UART1_RX")) {
            return -1;
        }
    }
    // if (port == "/dev/ttyS1") { // Check if the UART port matches
    //     auto ret = peripheral::pinmap::set_pin_function("A19", "UART1_TX"); // Set pin function for UART TX
    //     if (ret != err::Err::ERR_NONE) {
    //         maix::log::error("pinmap error"); // Log error if pin mapping fails
    //         return -1; // Exit with error
    //     }
    //     ret = peripheral::pinmap::set_pin_function("A18", "UART1_RX"); // Set pin function for UART RX
    //     if (ret != err::Err::ERR_NONE) {
    //         maix::log::error("pinmap error"); // Log error if pin mapping fails
    //         return -1; // Exit with error
    //     }
    // }

    /* Initialize motors */
    ext_dev::tmc2209::ScrewSlide slide_x(port.c_str(), uart_addr_x, uart_baudrate,
                            step_angle_x, micro_step_x, screw_pitch_x, speed_x); // Initialize motor X

    ext_dev::tmc2209::ScrewSlide slide_y(port.c_str(), uart_addr_y, uart_baudrate,
                            step_angle_y, micro_step_y, screw_pitch_y, speed_y); // Initialize motor Y

    bool need_exit = false; // Flag to exit gracefully
    auto one_step_callback = [&cam, &disp, &need_exit]() {
        auto img = cam.read(); // Read an image from the camera
        disp.show(*img);       // Display the image
        delete img;           // Free the image memory
        return need_exit;      // Return exit flag
    };

    // Launch video processing in a separate thread if video mode is selected
    if constexpr (MODE == MODE_SHOW_VIDEO) {
        std::thread([&one_step_callback](){
            while (!app::need_exit()) {
                if (one_step_callback()) break; // Call callback to process one step
            }
        }).detach(); // Detach the thread for concurrent execution
    }

    // Initialize current positions based on directions
    auto current_x = dir_x ? start_x : stop_x; // Current position for motor X
    auto current_y = dir_y ? start_y : stop_y; // Current position for motor Y

    auto dir_x_ = dir_x; // Copy direction for motor X

    // Loop to move motor Y
    for (int i = 0; i < (stop_y - start_y) / step_y + 1; ++i) {
        auto target_move_y = (i != 0) ? step_y : 0; // Determine target move for motor Y
        target_move_y = (dir_y) ? target_move_y : -target_move_y; // Adjust direction
        slide_y.move(target_move_y); // Move motor Y
        current_y = (dir_y) ? (current_y + step_y) : (current_y - step_y); // Update current position for motor Y

        // Loop to move motor X
        for (int j = 0; j < (stop_x - start_x) / step_x; ++j) {
            auto target_move_x = (dir_x_) ? step_x : -step_x; // Determine target move for motor X
            slide_x.move(target_move_x); // Move motor X
            current_x += target_move_x; // Update current position for motor X
            if constexpr (MODE == MODE_SHOW_PICTURE)
                one_step_callback(); // Call the callback to show image if in picture mode
            if (app::need_exit()) break; // Check for exit condition
        }
        dir_x_ = (dir_x_) ? false : true; // Reverse direction for motor X
        if (app::need_exit()) break; // Check for exit condition
    }

    need_exit = true; // Set exit flag to true
    /* Wait for camera thread exit */
    maix::time::sleep(3); // Sleep for 3 seconds to ensure all processing is complete

    return 0; // Exit the program successfully

}

int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}


