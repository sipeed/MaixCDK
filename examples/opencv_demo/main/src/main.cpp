
#include "maix_basic.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/freetype.hpp"
#include "main.h"

using namespace std;
using namespace maix;

uint64_t t = 0;
#define t_start() t = time::ticks_us()
#define t_end(msg) do{log::info("%s cost: %d us", msg, time::ticks_us() - t);}while(0)
#define t_end_start(msg) do{log::info("%s cost: %d us", msg, time::ticks_us() - t);t_start();}while(0)

void opencv_ops(cv::Mat &rgb)
{
    static char uart_buff[1024] = {0};

    cv::Mat gray;
    t_start();
    cv::cvtColor(rgb, gray, cv::COLOR_RGB2GRAY);
    t_end_start("cvtColor RGB2GRAY");
    cv::Mat blur;
    cv::GaussianBlur(gray, blur, cv::Size(5, 5), 0, 0);
    t_end_start("GaussianBlur");
    // binary image
    cv::Mat binary;
    cv::threshold(blur, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    t_end_start("threshold");
    // invert image
    cv::Mat invert;
    cv::bitwise_not(binary, invert);
    t_end_start("bitwise_not");
    // find contours
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(invert, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    t_end_start("findContours");
    // get the largets contour
    int largest_area = 0;
    int largest_contour_index = 0;
    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        if (area > largest_area)
        {
            largest_area = area;
            largest_contour_index = i;
        }
    }
    t_end_start("largest_contour");
    // 多边形包围最大轮廓
    std::vector<cv::Point> poly;
    cv::approxPolyDP(contours[largest_contour_index], poly, 3, true);
    t_end_start("approxPolyDP");
    // 从 poly 找到凸出的点
    std::vector<cv::Point> hull;
    cv::convexHull(poly, hull);
    t_end_start("convexHull");


    // find red point on rgb image
    cv::Mat hsv;
    cv::cvtColor(rgb, hsv, cv::COLOR_RGB2HSV);
    t_end_start("cvtColor RGB2HSV");
    cv::Mat mask, mask2;
    cv::inRange(hsv, cv::Scalar(0, 43, 46), cv::Scalar(10, 255, 255), mask);
    cv::inRange(hsv, cv::Scalar(156, 43, 46), cv::Scalar(180, 255, 255), mask2);
    mask = mask | mask2;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    t_end_start("morphologyEx");
    // find biggest contour on mask
    std::vector<std::vector<cv::Point>> contours2;
    std::vector<cv::Vec4i> hierarchy2;
    cv::findContours(mask, contours2, hierarchy2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    t_end_start("findContours");
    // get the largets contour
    int largest_area2 = 0;
    int largest_contour_index2 = 0;
    for (size_t i = 0; i < contours2.size(); i++)
    {
        double area = cv::contourArea(contours2[i]);
        if (area > largest_area2)
        {
            largest_area2 = area;
            largest_contour_index2 = i;
        }
    }

    t_end_start("largest_contour");

    // find green point on rgb image
    cv::Mat mask_green;
    cv::inRange(hsv, cv::Scalar(35, 43, 46), cv::Scalar(77, 255, 255), mask_green);
    cv::morphologyEx(mask_green, mask_green, cv::MORPH_OPEN, kernel);
    // find biggest contour on mask
    std::vector<std::vector<cv::Point>> contours_green;
    std::vector<cv::Vec4i> hierarchy_green;
    cv::findContours(mask_green, contours_green, hierarchy_green, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    t_end_start("findContours");
    // get the largets contour
    int largest_area_green = 0;
    int largest_contour_index_green = 0;
    for (size_t i = 0; i < contours_green.size(); i++)
    {
        double area = cv::contourArea(contours_green[i]);
        if (area > largest_area_green)
        {
            largest_area_green = area;
            largest_contour_index_green = i;
        }
    }

    t_end_start("largest_contour");

    // draw points
    // if(hull.size() == 4) // 只在有4个点的时候显示
    {
        // 在 rgb 图上画出凸包
        cv::polylines(rgb, hull, true, cv::Scalar(0, 255, 0), 2);
        // 在 rgb 图上画出 hull 点
        for (size_t i = 0; i < hull.size(); i++)
        {
            cv::circle(rgb, hull[i], 5, cv::Scalar(0, 0, 255), 2);
        }
    }
    t_end_start("draw points");

    // get the center point of the largest contour
    cv::Point2f mc, mc_green;
    if(contours2.size() > 0)
    {
        cv::Moments mu = cv::moments(contours2[largest_contour_index2], false);
        mc = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
        cv::circle(rgb, mc, 5, cv::Scalar(255, 0, 255), 2);
    }
    if(contours_green.size() > 0)
    {
        cv::Moments mu = cv::moments(contours_green[largest_contour_index_green], false);
        mc_green = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
        cv::circle(rgb, mc_green, 5, cv::Scalar(255, 255, 0 ), 2);
    }
    t_end_start("get center point");

    // save image
    cv::imwrite("/tmp/opencv_demo_out.png", rgb);
    t_end_start("save png");
    cv::imwrite("/tmp/opencv_demo_out.jpg", rgb);
    t_end_start("save jpg");

    // send result(center point, points) to uart
    snprintf(uart_buff, sizeof(uart_buff), "red: %d, %d, green: %d, %d, points: %d, ",
                    (int)mc.x, (int)mc.y, (int)mc_green.x, (int)mc_green.y, (int)hull.size());
    for(size_t i = 0; i < hull.size(); i++)
    {
        snprintf(uart_buff + strlen(uart_buff), sizeof(uart_buff) - strlen(uart_buff), "%d, %d, ", (int)hull[i].x, (int)hull[i].y);
    }
    snprintf(uart_buff + strlen(uart_buff), sizeof(uart_buff) - strlen(uart_buff), "\r\n");
    log::info("%s", uart_buff);
}

void opencv_test(string &img_path)
{
    // read
    uint64_t t = time::ticks_ms();
    cv::Mat img = cv::imread(img_path);
    log::info("read image cost: %d ms", time::ticks_ms() - t);
    if (img.empty())
    {
        printf("read image failed: %s\n", img_path.c_str());
        return;
    }
    t = time::ticks_ms();
    // draw rectangle on center
    cv::rectangle(img, cv::Point(img.cols/4, img.rows/4), cv::Point(img.cols*3/4, img.rows*3/4), cv::Scalar(0, 0, 255), 2);
    log::info("draw rectangle cost: %d ms", time::ticks_ms() - t);
    // save to file
    string out_path = img_path + ".out.png";
    t = time::ticks_ms();
    cv::imwrite(out_path, img);
    log::info("save image cost: %d ms", time::ticks_ms() - t);

    t = time::ticks_ms();
    opencv_ops(img);
    log::info("opencv ops cost: %d ms", time::ticks_ms() - t);
}


int _main(int argc, char *argv[])
{
    // read image path from argv
    string img_path = "";
    if (argc > 1)
    {
        img_path = argv[1];
    }
    else
    {
        printf("Usage: %s <image path>\n", argv[0]);
        return 0;
    }
    opencv_test(img_path);
    return 0;
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
