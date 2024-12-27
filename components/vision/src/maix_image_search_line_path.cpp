#include "maix_image.hpp"
#include "maix_time.hpp"
#include "maix_image_util.hpp"
#include "opencv2/opencv.hpp"
#include <vector>
#include <iostream>
#include <list>

using namespace maix;

#define __DEBUG     (0)
#if __DEBUG
#define DEBUG_EN(x)                                                         \
    bool g_debug_flag = x;
#define DEBUG_IS_ENABLE() g_debug_flag
#define DEBUG_PRT(fmt, ...) do {                                            \
    if (g_debug_flag)                                                       \
        printf("[%s][%d]: " fmt "\r\n", __func__, __LINE__, ##__VA_ARGS__);   \
} while(0)
#define DEBUG_PRT0(fmt, ...) do {                                            \
    if (g_debug_flag)                                                       \
        printf(fmt, __func__, __LINE__, ##__VA_ARGS__);   \
} while(0)
#define DEBUG_WAIT(en) do { \
    if (en) {               \
        printf("Press enter to continue...\r\n"); \
        std::cin.get();     \
    }                       \
} while (0)
#else
#define DEBUG_EN(fmt, ...)
#define DEBUG_PRT(fmt, ...)
#define DEBUG_PRT0(fmt, ...)
#define DEBUG_IS_ENABLE(fmt, ...)
#endif

#if __DEBUG
image::Color debug_get_color()
{
    int static color_count = 0;
    color_count += 1;
    switch (color_count) {
    case 0: return image::COLOR_BLUE;
    case 1: return image::COLOR_GREEN;
    case 2: return image::COLOR_PURPLE;
    case 3: return image::COLOR_ORANGE;
    case 4: return image::COLOR_GRAY;
    default:
        color_count = 0;
        return image::COLOR_BLUE;
    }
}
#endif

namespace maix::image {
typedef struct {
    int x, y;
} point_t;

static void sort_vertical_point(cv::Point start, cv::Point end, cv::Point &top, cv::Point &bottom) {
    if (start.y <= end.y) {
        top = start;
        bottom = end;
    } else {
        top = end;
        bottom = start;
    }
}

static void sort_horizontal_point(cv::Point start, cv::Point end, cv::Point &left, cv::Point &right) {
    if (start.x <= end.x) {
        left = start;
        right = end;
    } else {
        left = end;
        right = start;
    }
}

static double compute_average_theta(double &theta1, double &theta2) {
    DEBUG_EN(0);
    DEBUG_PRT("theta1:%f theta2:%f", theta1, theta2);
    if (std::abs(theta2 - theta1) > M_PI / 2) {
        return (theta1 + theta2 + M_PI) / 2;
    } else {
        return (theta1 + theta2) / 2;
    }
}

static bool is_vertical(double angle)
{
    if ((0 <= angle && angle <= 45) || (135 <= angle && angle <= 180)) {
        return true;
    } else {
        return false;
    }
}

#if 0
static void found_the_y_of_max_and_min_value(std::vector<cv::Point> &points, cv::Point &min, cv::Point &max) {
    int min_y = 100000, max_y = 0;
    int x_from_min_y = 0, x_from_max_y = 0;
    for (size_t i = 0; i < points.size(); i ++) {
        if (points[i].y < min_y) {
            min_y = points[i].y;
            x_from_min_y = points[i].x;
        } else if (points[i].y == min_y) {
            if (points[i].x < x_from_min_y) {
                x_from_min_y = points[i].x;
            }
        }

        if (points[i].y > max_y) {
            max_y = points[i].y;
            x_from_max_y = points[i].x;
        } else if (points[i].y == min_y) {
            if (points[i].x > x_from_max_y) {
                x_from_max_y = points[i].x;
            }
        }
    }

    min = {x_from_min_y, min_y};
    max = {x_from_max_y, max_y};
}
#endif
class NewLine {
public:
    cv::Point start;
    cv::Point end;
    cv::Point center;
    double magnitude;
    double theta;
    double rho;
    double A;
    double B;
    double C;
    double m;
    double b;
    int sum_n = 0;
    double sum_x = 0;
    double sum_y = 0;
    double sum_xx = 0;
    double sum_xy = 0;

    void push_new_point(cv::Point &point) {
        DEBUG_EN(0);
        sum_x += point.x;
        sum_y += point.y;
        sum_xx += point.x * point.x;
        sum_xy += point.x * point.y;
        sum_n += 1;
        DEBUG_PRT("sum_n:%d sum_x:%f sum_y:%f sum_xx:%f sum_xy:%f, point(%d,%d)", sum_n, sum_x, sum_y, sum_xx, sum_xy, point.x, point.y);
    }

    void compute_m_and_b(double &m, double &b) {
        DEBUG_EN(0);
        m = (sum_n * sum_xy - sum_x * sum_y) / (sum_n * sum_xx - sum_x * sum_x);
        b = (sum_y - m * sum_x) / sum_n;
        DEBUG_PRT("compute m:%f b:%f", m, b);
    }

    void compute_x_with_m_and_b(int y, int &x) {
        DEBUG_EN(0);
        x = (y - b) / m;
        DEBUG_PRT("input y:%d, x = (%d - (%f)) / (%f) = %d", y, y, b, m, x);
    }

    void compute_y_with_m_and_b(int x, int &y) {
        DEBUG_EN(0);
        y = m * x + b;
        DEBUG_PRT("input x:%d, y = (%d * (%f)) + (%f) = %d", x, x, m, b, y);
    }

    static int get_points_distance2(point_t point1, point_t point2)
    {
        return std::sqrt(std::pow(point2.x - point1.x, 2) + std::pow(point2.y - point1.y, 2));
    }

    static double slope_to_angle(double slope) {
        return atan(slope) * (180.0 / M_PI);
    }

    static double calculate_angle(point_t point1, point_t point2)
    {
        if (point2.x == point1.x) {
            return 90;
        }

        double k = (double)(point2.y - point1.y) / (point2.x - point1.x);
        auto angle = slope_to_angle(k);
        return angle;
    }

    static void calculate_magnitude_theta_and_rho(point_t p1, point_t p2, double &A, double &B, double &C, double &magnitude, double &theta, double &rho)
    {
        DEBUG_EN(0);
        // Ax + By + C = 0
        A = p2.y - p1.y;
        B = p1.x - p2.x;
        C = p2.x * p1.y - p1.x * p2.y;

        rho = - C / sqrt(A * A + B * B);
        if (B == 0) {
            theta = 0;
        } else {
            theta = atan2(B, A);
        }

        DEBUG_PRT("p1(%d,%d) p2(%d,%d)", p1.x, p1.y, p2.x, p2.y);
        DEBUG_PRT("A:%f B:%f C:%f theta:%f rho:%f", A, B, C, theta, rho);
    }

    NewLine() {

    }
    NewLine(cv::Point &start_point, cv::Point &end_point) {
        start = start_point;
        end = end_point;
        center = cv::Point(start.x + (end.x - start.x) / 2, start.y + (end.y - start.y) / 2);
        calculate_magnitude_theta_and_rho({start.x, start.y}, {end.x, end.y}, A, B, C, magnitude, theta, rho);
    }

    double degree(bool use_other_theta = false, double other_theta = -1) {
        double new_theta = 0;
        if (use_other_theta) {
            new_theta = other_theta;
        } else {
            new_theta = theta;
        }
        if (new_theta < 0) {
            new_theta += M_PI;
        }
        auto degree = new_theta * 180.0 / M_PI;
        return degree;
    }

    int center_distance(int cx, int cy) {
        return get_points_distance2({cx, cy}, {center.x, center.y});
    }

    int point_distance(int x, int y) {
        auto distance = (int)(fabs(A * x + B * y + C) / sqrt(A * A + B * B));
        return distance;
    }

    int point_distance_without_abs(int x, int y) {
        auto distance = (int)(A * x + B * y + C) / sqrt(A * A + B * B);
        return distance;
    }

    bool compute_x(int y, int &x, bool use_new_param = false, double new_theta = -1, double new_rho = -1) {
        DEBUG_EN(0);
        double t, r;
        if (use_new_param) {
            t = new_theta;
            r = new_rho;
        } else {
            t = theta;
            r = rho;
        }
        float sin_theta = std::sin(t);
        float cos_theta = std::cos(t);
        if (std::abs(sin_theta) < 1e-6) {
            return false;
        }
        DEBUG_PRT("A:%f B:%f C:%f theta:%f cos(theta):%f sin(theta):%f rho:%f", A, B, C, t, cos(t), sin(t), r);
        x = (r - y * sin_theta) / cos_theta;
        DEBUG_PRT("input x:%d theta:%f rho:%f cos_theta:%f sin_theta:%f output y:%d", x, t, r, cos_theta, sin_theta, y);
        return true;
    }

    bool compute_y(int x, int &y, bool use_new_param = false, double new_theta = -1, double new_rho = -1) {
        DEBUG_EN(0);
        double t, r;
        if (use_new_param) {
            t = new_theta;
            r = new_rho;
        } else {
            t = theta;
            r = rho;
        }
        float sin_theta = std::sin(t);
        float cos_theta = std::cos(t);
        if (std::abs(sin_theta) < 1e-6) {
            return false;
        }
        DEBUG_PRT("A:%f B:%f C:%f theta:%f cos(theta):%f sin(theta):%f rho:%f", A, B, C, t, cos(t), sin(t), r);
        y = (r - x * cos_theta) / sin_theta;
        DEBUG_PRT("input x:%d theta:%f rho:%f cos_theta:%f sin_theta:%f output y:%d", x, t, r, cos_theta, sin_theta, y);
        return true;
    }

    bool compute_intersection(NewLine &line, int &x, int &y) {
        if (line.theta == theta) {
            return false;
        }

        double denominator = A * line.B - line.A * B;
        x = (B * line.C - line.B * C) / denominator;
        y = (line.A * C - A * line.C) / denominator;
        return true;
    }

    void merge_line(NewLine &line) {
        DEBUG_EN(0);
        DEBUG_PRT("line1 start:(%d,%d) end:(%d,%d) theta:%f rho:%f degree:%f", start.x, start.y, end.x, end.y, theta, rho, degree());
        DEBUG_PRT("line2 start:(%d,%d) end:(%d,%d) theta:%f rho:%f degree:%f", line.start.x, line.start.y, line.end.x, line.end.y, line.theta, line.rho, line.degree());

        auto average_theta = compute_average_theta(theta, line.theta);
        auto average_rho = (rho + line.rho) / 2;
        auto average_degree = degree(true, average_theta);
        cv::Point new_start = {0}, new_end = {0};
        float max_rotation_degree = 5;
        DEBUG_PRT("average theta:%f average rho:%f average degree:%f", average_theta, average_rho, average_degree);
        if (is_vertical(average_degree)) {     // vertical
            DEBUG_PRT("merged line is [vertival], average_degree:%f", average_degree);
            cv::Point top1 = {0}, bottom1 = {0}, top2 = {0}, bottom2 = {0};
            sort_vertical_point(start, end, top1, bottom1);
            sort_vertical_point(line.start, line.end, top2, bottom2);
            DEBUG_PRT("top1 (%d,%d) bottom1 (%d,%d) top2 (%d,%d) bottom2 (%d,%d)", top1.x, top1.y, bottom1.x, bottom1.y, top2.x, top2.y, bottom2.x, bottom2.y);
            int min_y = std::min(top1.y, top2.y);
            int max_y = std::max(bottom1.y, bottom2.y);
            if (average_degree <= max_rotation_degree || average_degree >= 180 - max_rotation_degree) {
                new_start.x = (bottom1.x + bottom2.x) / 2;
                new_start.y = max_y;
                new_end.x = (top1.x + top2.x) / 2;
                new_end.y = min_y;
            } else {
                int top_x = 0, bottom_x = 0;
                compute_x(min_y, top_x, true, average_theta, average_rho);
                compute_x(max_y, bottom_x, true, average_theta, average_rho);
                new_start.x = bottom_x;
                new_start.y = max_y;
                new_end.x = top_x;
                new_end.y = min_y;
            }
        } else {                                            // horizontal
            DEBUG_PRT("merged line is [hotizontal], average_degree:%f(cos_theta:%f)", average_degree, std::abs(cos_theta));
            cv::Point left1 = {0}, right1 = {0}, left2 = {0}, right2 = {0};
            sort_horizontal_point(start, end, left1, right1);
            sort_horizontal_point(line.start, line.end, left2, right2);
            DEBUG_PRT("left1 (%d,%d) right1 (%d,%d) left2 (%d,%d) right2 (%d,%d)", left1.x, left1.y, right1.x, right1.y, left2.x, left2.y, right2.x, right2.y);
            int min_x = std::min(left1.x, left2.x);
            int max_x = std::max(right1.x, right2.x);
            if (90 - max_rotation_degree <= average_degree && average_degree <= 90 + max_rotation_degree) {
                new_start.x = min_x;
                new_start.y = (left1.y + left2.y) / 2;
                new_end.x = max_x;
                new_end.y = (right1.y + right2.y) / 2;
            } else {
                int left_y = 0, right_y = 0;
                compute_y(min_x, left_y, true, average_theta, average_rho);
                compute_y(max_x, right_y, true, average_theta, average_rho);
                new_start.x = min_x;
                new_start.y = left_y;
                new_end.x = max_x;
                new_end.y = right_y;
            }
        }

        start = new_start;
        end = new_end;

        // FIXME:
        if ((B > 0 && start.x - end.x < 0) || (B < 0 && start.x - end.x > 0))
        {
            auto tmp = end;
            end = start;
            start = tmp;
        }

        center = cv::Point(center.x + (end.x - start.x) / 2, start.y + (end.y - start.y) / 2);
        calculate_magnitude_theta_and_rho({start.x, start.y}, {end.x, end.y}, A, B, C, magnitude, theta, rho);
        DEBUG_PRT("merged line start:(%d,%d) end:(%d,%d) theta:%f rho:%f degree:%f", start.x, start.y, end.x, end.y, theta, rho, degree());
    }

    bool check_lower_line(NewLine &line) {
        if (start.y < line.start.y
            || start.y < line.end.y
            || end.y < line.start.y
            || end.y < line.end.y) {
            return true;
        } else {
            return false;
        }
    }

    image::LineType get_lines_type(NewLine &line, int min_diff_of_degree = 70, int max_point_to_line_distance = 10) {
        DEBUG_EN(0);
        DEBUG_PRT("l1 (%d,%d) (%d,%d)", start.x, start.y, end.x, end.y);
        DEBUG_PRT("l2 (%d,%d) (%d,%d)", line.start.x, line.start.y, line.end.x, line.end.y);
        DEBUG_PRT("l1 degree:%f l2 degree:%f min_diff_of_degree:%d max_point_to_line_distance:%d", degree(), line.degree(), min_diff_of_degree, max_point_to_line_distance);
        if (abs(line.degree() - degree()) >= min_diff_of_degree) {
            auto distance1 = point_distance_without_abs(line.start.x, line.start.y);
            auto distance2 = point_distance_without_abs(line.end.x, line.end.y);
            auto distance3 = line.point_distance_without_abs(start.x, start.y);
            auto distance4 = line.point_distance_without_abs(end.x, end.y);
            DEBUG_PRT("distance1:%d distance2:%d distance3:%d distance4:%d", distance1, distance2, distance3, distance4);
            if ((distance1 <= max_point_to_line_distance
                    || distance2 <= max_point_to_line_distance)
                    && (distance3 <= max_point_to_line_distance
                    || distance4 <= max_point_to_line_distance)) {
                return image::LineType::LINE_L;
            } else if (distance1 <= max_point_to_line_distance
                    || distance2 <= max_point_to_line_distance
                    || distance3 <= max_point_to_line_distance
                    || distance4 <= max_point_to_line_distance) {
                return image::LineType::LINE_T;
            } else {
                return image::LineType::LINE_CROSS;
            }
        } else {
            return image::LineType::LINE_NORMAL;
        }
    }
};

static image::LineType get_group_type(NewLine &front_line, NewLine &back_line, int min_diff_of_degree = 70, int max_point_to_line_distance = 10)
{
    NewLine ver_line, hor_line;
    // Check the straight line towards the bottom of the screen
    if (is_vertical(front_line.degree())) {
        ver_line = front_line;
        hor_line = back_line;
    } else {
        ver_line = back_line;
        hor_line = front_line;
    }

    cv::Point top, bottom, left, right;;
    sort_vertical_point(ver_line.start, ver_line.end, top, bottom);
    sort_horizontal_point(hor_line.start, hor_line.end, left, right);

    // compute intersection point
    point_t intersection_point = {0, 0};
    ver_line.compute_intersection(hor_line, intersection_point.x, intersection_point.y);

    DEBUG_EN(0);
    DEBUG_PRT("ver_line (%d,%d) (%d,%d)", top.x, top.y, bottom.x, bottom.y);
    DEBUG_PRT("hor_line (%d,%d) (%d,%d)",left.x, left.y, right.x, right.y);
    DEBUG_PRT("ver_line degree:%f hor_line degree:%f min_diff_of_degree:%d max_point_to_line_distance:%d", ver_line.degree(), hor_line.degree(), min_diff_of_degree, max_point_to_line_distance);
    if (abs(ver_line.degree() - hor_line.degree()) >= min_diff_of_degree) {
        auto left_to_ver_dist = ver_line.point_distance_without_abs(left.x, left.y);
        auto right_to_ver_dist = ver_line.point_distance_without_abs(right.x, right.y);
        auto top_to_hor_dist = hor_line.point_distance_without_abs(top.x, top.y);
        auto bottom_to_hor_dist = hor_line.point_distance_without_abs(bottom.x, bottom.y);

        // FIXME:
        if (right.x > intersection_point.x) {
            right_to_ver_dist = right_to_ver_dist > 0 ? -right_to_ver_dist : right_to_ver_dist;
        }

        if (left.x < intersection_point.x) {
            left_to_ver_dist = left_to_ver_dist < 0 ? -left_to_ver_dist : left_to_ver_dist;
        }
        DEBUG_PRT("left_to_ver_dist:%d right_to_ver_dist:%d top_to_hor_dist:%d bottom_to_hor_dist:%d", left_to_ver_dist, right_to_ver_dist, top_to_hor_dist, bottom_to_hor_dist);

        if (right_to_ver_dist <= (-max_point_to_line_distance) && left_to_ver_dist >= max_point_to_line_distance && top_to_hor_dist >= max_point_to_line_distance && bottom_to_hor_dist <= (-max_point_to_line_distance)) {
            /**
             *     |
             * ----|----
             *     |
            */
            return image::LineType::LINE_CROSS;
        }  else if (
            /**
             * -------
             *    |
             *    |
            */
            (left_to_ver_dist >= max_point_to_line_distance && right_to_ver_dist <= (-max_point_to_line_distance) && top_to_hor_dist <= max_point_to_line_distance)
            /**
             *    |
             *    |
             * -------
            */
            || (left_to_ver_dist >= max_point_to_line_distance && right_to_ver_dist <= (-max_point_to_line_distance) && bottom_to_hor_dist >= (-max_point_to_line_distance))
            /**
             * |
             * |------
             * |
            */
            || (left_to_ver_dist <= max_point_to_line_distance && top_to_hor_dist >= max_point_to_line_distance && bottom_to_hor_dist <= (-max_point_to_line_distance))
            /**
             *       |
             * ------|
             *       |
            */
            || (right_to_ver_dist >= (-max_point_to_line_distance) && top_to_hor_dist >= max_point_to_line_distance && bottom_to_hor_dist <= (-max_point_to_line_distance))) {
            return image::LineType::LINE_T;
        } else if (
            /**
             * -----
             * |
             * |
            */
            (left_to_ver_dist <= max_point_to_line_distance && top_to_hor_dist <= max_point_to_line_distance)
            /**
             * |
             * |
             * -----
            */
            || (left_to_ver_dist <= max_point_to_line_distance && bottom_to_hor_dist >= (-max_point_to_line_distance))
            /**
             * -----
             *     |
             *     |
            */
            || (right_to_ver_dist >= (-max_point_to_line_distance) && top_to_hor_dist <= max_point_to_line_distance)
            /**
             *     |
             *     |
             * -----
            */
            || (right_to_ver_dist >= (-max_point_to_line_distance) && bottom_to_hor_dist >= (-max_point_to_line_distance))) {
            return image::LineType::LINE_L;
        } else {
            return image::LineType::LINE_NORMAL;
        }
    } else {
        return image::LineType::LINE_NORMAL;
    }
}

static std::vector<image::LineGroup> found_path(std::list<NewLine> &merged_lines, int min_len_of_new_path)
{
    DEBUG_EN(0);
    std::vector<image::LineGroup> groups;
    auto group_type = image::LineType::LINE_NORMAL;
    int group_idx = 0;
    if (merged_lines.size() == 2) {
        auto front_line = merged_lines.front();
        auto back_line = merged_lines.back();
        group_idx = 0;
        group_type = get_group_type(front_line, back_line, 70, min_len_of_new_path);
        std::vector<image::Line> lines;

        switch (group_type) {
        case image::LineType::LINE_L:
        {
            NewLine ver_line, hor_line;
            // Check the straight line towards the bottom of the screen
            if (is_vertical(front_line.degree())) {
                ver_line = front_line;
                hor_line = back_line;
            } else {
                ver_line = back_line;
                hor_line = front_line;
            }

            // compute intersection point
            point_t intersection_point = {0, 0};
            ver_line.compute_intersection(hor_line, intersection_point.x, intersection_point.y);

            // push vertical line
            cv::Point start, end;
            sort_vertical_point(ver_line.start, ver_line.end, start, end);
            end = {intersection_point.x, intersection_point.y};
            lines.push_back(image::Line(start.x, start.y, end.x, end.y, ver_line.magnitude, ver_line.theta, ver_line.rho));

            // push horizontal line
            start = {intersection_point.x, intersection_point.y};
            auto distance1 = ver_line.point_distance(hor_line.start.x, hor_line.start.y);
            auto distance2 = ver_line.point_distance(hor_line.end.x, hor_line.end.y);
            if (distance1 >= distance2) {
                end = {hor_line.start.x, hor_line.start.y};
            } else {
                end = {hor_line.end.x, hor_line.end.y};
            }
            lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
        }
        break;
        case image::LineType::LINE_T:
        {
            NewLine ver_line, hor_line;
            // Check the straight line towards the bottom of the screen
            if (is_vertical(front_line.degree())) {
                ver_line = front_line;
                hor_line = back_line;
            } else {
                ver_line = back_line;
                hor_line = front_line;
            }

            // compute intersection point
            point_t intersection_point = {0, 0};
            ver_line.compute_intersection(hor_line, intersection_point.x, intersection_point.y);

            cv::Point top, bottom, left, right;;
            sort_vertical_point(ver_line.start, ver_line.end, top, bottom);
            sort_horizontal_point(hor_line.start, hor_line.end, left, right);
            if (hor_line.point_distance(top.x, top.y) <= min_len_of_new_path) {
                /**
                 * -------
                 *    |
                */
                // push vertical line
                lines.push_back(image::Line(bottom.x, bottom.y, intersection_point.x, intersection_point.y, ver_line.magnitude, ver_line.theta, ver_line.rho));

                // push horizontal line
                lines.push_back(image::Line(left.x, left.y, intersection_point.x, intersection_point.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                lines.push_back(image::Line(intersection_point.x, intersection_point.y, right.x, right.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
            } else if (hor_line.point_distance(bottom.x, bottom.y) <= min_len_of_new_path) {
                /**
                 *    |
                 * -------
                */
                // push vertical line
                lines.push_back(image::Line(intersection_point.x, intersection_point.y, top.x, top.y, ver_line.magnitude, ver_line.theta, ver_line.rho));

                // push horizontal line
                lines.push_back(image::Line(left.x, left.y, intersection_point.x, intersection_point.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                lines.push_back(image::Line(intersection_point.x, intersection_point.y, right.x, right.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
            } else {
                /**
                 *      |    |
                 *  ----| or |----
                 *      |    |
                */
                // push vertical line
                lines.push_back(image::Line(top.x, top.y, intersection_point.x, intersection_point.y, ver_line.magnitude, ver_line.theta, ver_line.rho));
                lines.push_back(image::Line(intersection_point.x, intersection_point.y, bottom.x, bottom.y, ver_line.magnitude, ver_line.theta, ver_line.rho));

                // push vertical line
                if (ver_line.point_distance(right.x, right.y) <= min_len_of_new_path) {
                    lines.push_back(image::Line(intersection_point.x, intersection_point.y, left.x, left.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                } else {
                    lines.push_back(image::Line(intersection_point.x, intersection_point.y, right.x, right.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                }
            }
        }
        break;
        case image::LineType::LINE_CROSS:
        {
            NewLine ver_line, hor_line;
            // Check the straight line towards the bottom of the screen
            if (is_vertical(front_line.degree())) {
                ver_line = front_line;
                hor_line = back_line;
            } else {
                ver_line = back_line;
                hor_line = front_line;
            }

            // compute intersection point
            point_t intersection_point = {0, 0};
            ver_line.compute_intersection(hor_line, intersection_point.x, intersection_point.y);

            // push vertical line
            cv::Point start, end;
            sort_vertical_point(ver_line.start, ver_line.end, start, end);
            lines.push_back(image::Line(start.x, start.y, intersection_point.x, intersection_point.y, ver_line.magnitude, ver_line.theta, ver_line.rho));
            lines.push_back(image::Line(intersection_point.x, intersection_point.y, end.x, end.y, ver_line.magnitude, ver_line.theta, ver_line.rho));
            DEBUG_PRT("ver line start(%d,%d) end(%d,%d)", ver_line.start.x, ver_line.start.y, ver_line.end.x, ver_line.end.y);
            DEBUG_PRT("ver sort line start(%d,%d) end(%d,%d)", start.x, start.y, end.x, end.y);

            // push horizontal line
            cv::Point left, right;
            sort_horizontal_point(hor_line.start, hor_line.end, left, right);
            lines.push_back(image::Line(left.x, left.y, intersection_point.x, intersection_point.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
            lines.push_back(image::Line(intersection_point.x, intersection_point.y, right.x, right.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
        }
        break;
        default:
            for (const auto& l : merged_lines) {
                cv::Point start, end;
                sort_vertical_point(l.start, l.end, start, end);
                lines.push_back(image::Line(start.x, start.y, end.x, end.y, l.magnitude, l.theta, l.rho));
            }
        break;
        }
        groups.push_back(image::LineGroup(group_idx, group_type, lines));
    } else {
        group_type = image::LineType::LINE_NORMAL;
        for (const auto& l : merged_lines) {
            std::vector<image::Line> lines;
            lines.push_back(image::Line(l.start.x, l.start.y, l.end.x, l.end.y, l.magnitude, l.theta, l.rho));
            groups.push_back(image::LineGroup(group_idx, group_type, lines));
            group_idx ++;
        }
    }

    return groups;
}

std::vector<image::LineGroup> Image::search_line_path(int threshold, int merge_degree, int min_len_of_new_path)
{
    DEBUG_EN(0);
    auto gray_img = (image::Image *)nullptr;
    auto need_free_gray_img = false;
    if (this->format() != image::FMT_GRAYSCALE) {
        // auto new_img = this->binary(thresholds, false, false, nullptr, false, true);
        // gray_img = new_img->to_format(image::FMT_GRAYSCALE);
        // delete new_img;

        gray_img = this->to_format(image::FMT_GRAYSCALE);
        need_free_gray_img = true;
    } else {
        // gray_img = this->binary(thresholds, false, false, nullptr, false, true);

        gray_img = this;
        need_free_gray_img = false;
    }

    cv::Mat edges;
    cv::Mat gray = cv::Mat(gray_img->height(), gray_img->width(), CV_8UC((int)image::fmt_size[gray_img->format()]), gray_img->data());

    // cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
    cv::Canny(gray, edges, 50, 150);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, 1, CV_PI / 180, threshold, 10, 100);

    std::list<NewLine> new_lines;
    std::list<NewLine> merged_lines;

    for (const auto& l : lines) {
        cv::Point start(l[0], l[1]);
        cv::Point end(l[2], l[3]);
        new_lines.push_back(NewLine(start, end));
    }

#if __DEBUG
    // this->save("/root/0.jpg");
    log::info(" ========== HoughLinesP found lines size:%ld ========== ", new_lines.size());
    for (auto it = new_lines.begin(); it != new_lines.end(); it ++) {
        auto line = *it;
        this->draw_line(line.start.x, line.start.y, line.end.x, line.end.y, image::COLOR_RED, 1);
    }
#endif

    while (!new_lines.empty()) {
        auto it1 = new_lines.begin();
        if (it1 == new_lines.end()) {
            break;
        }

        auto line1 = *it1;
        DEBUG_PRT(" ========== merge line[%ld] line1 degree:%f ========== ", merged_lines.size(), line1.degree());
        for (auto it2 = std::next(it1); it2 != new_lines.end();) {
            auto line2 = *it2;
            auto line1_degree = line1.degree();
            auto line2_degree = line2.degree();
            auto degree_diff = fabs(line1_degree - line2_degree);
            DEBUG_PRT("[CHECK] degree1:%f, degree2:%f degree diff:%f merge_degree:[0, %d] or [%d, 180]", line1_degree, line2_degree, fabs(line1_degree - line2_degree), merge_degree, 180 - merge_degree);
            if (degree_diff <= merge_degree || (degree_diff >= 180 - merge_degree && merge_degree <= 180)) {
                line1.merge_line(line2);
                it2 = new_lines.erase(it2);
            } else {
                it2 ++;
            }
        }

        merged_lines.push_back(line1);
        new_lines.erase(it1);
    }

#if __DEBUG
    log::info(" ========== Merged lines size:%d ========== ", merged_lines.size());
    for (auto it = merged_lines.begin(); it != merged_lines.end(); it ++) {
        auto line = *it;
        this->draw_line(line.start.x, line.start.y, line.end.x, line.end.y, image::COLOR_GREEN, 1);
    }
#endif

    auto groups = found_path(merged_lines, min_len_of_new_path);

    if (need_free_gray_img) {
        delete gray_img;
    }

    return groups;
}
}
