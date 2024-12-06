#include "maix_image.hpp"
#include "maix_time.hpp"
#include "maix_image_util.hpp"
#include <vector>
#include <list>

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
#else
#define DEBUG_EN(fmt, ...)
#define DEBUG_PRT(fmt, ...)
#define DEBUG_PRT0(fmt, ...)
#define DEBUG_IS_ENABLE(fmt, ...)
#endif

namespace maix::image {

typedef struct {
    int x, y;
} point_t;

typedef struct {
    point_t p1, p2;
} line_t;

class LinePoint : public std::list<point_t> {
    int _sum_x = 0;
    int _sum_y = 0;
    int _sum_xy = 0;
    int _sum_xx = 0;
    double _angle;
    double _m;
    double _b;
    point_t _first_point;
    std::list<LinePoint> bifurcation_lines;
public:
    void push_back(const point_t& p) {
        if (empty()) {
            _first_point = p;
        }
        std::list<point_t>::push_back(p);

        _sum_x += p.x;
        _sum_y += p.y;
        _sum_xy += p.x * p.y;
        _sum_xx += p.x * p.x;

        calculate_m_and_b(size(), _sum_x, _sum_y, _sum_xx, _sum_xy, _m, _b, _angle);
    }

    bool check_point_is_valid(point_t new_point, double max_angle, double max_distance) {
        DEBUG_EN(0);
        DEBUG_PRT("check new point(%d,%d) max angle:%f", new_point.x, new_point.y, max_angle);
        if (size() <= 1) return true;

        auto tmp_sum_x = _sum_x + new_point.x;
        auto tmp_sum_y = _sum_y + new_point.y;
        auto tmp_sum_xy = _sum_xy + new_point.x * new_point.y;
        auto tmp_sum_xx = _sum_xx + new_point.x * new_point.x;

        double m = 0, b = 0, angle = 0;
        calculate_m_and_b(size() + 1, tmp_sum_x, tmp_sum_y, tmp_sum_xx, tmp_sum_xy, m, b, angle);

        double distance = point_to_line_distance(new_point);
        DEBUG_PRT("angle of line:%f, max distance %f, _m:%f _b:%f new point angle:%f distance:%f", _angle, max_distance, _m, _b, angle, distance);
        if (abs(angle - _angle) <= max_angle && distance < max_distance) return true;
        return false;
    }

    double point_to_line_distance(point_t &p) {
        if ((int)_angle == 90) {
            return fabs(p.x - _b);
        } else {
            return fabs(_m * p.x - p.y + _b) / std::sqrt(_m * _m + 1);
        }
    }

    static void calculate_m_and_b(int n, int sum_x, int sum_y, int sum_xx, int sum_xy, double &m, double &b, double &angle)
    {
        if (n * sum_xx - sum_x * sum_x == 0) {
            m = 0;          // invalid
            b = sum_x / n;
            angle = 90;
        } else {
            m = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
            b = ((double)sum_y - m * sum_x) / n;
            angle = atan(m) * (180.0 / M_PI);
        }
    }

    double m() {return _m;}
    double b() {return _b;}

    void add_bifurcation_line(LinePoint &line) {bifurcation_lines.push_back(line);}
    auto get_bifurcation_lines() {return bifurcation_lines;}

    auto first_point() {return _first_point;}
};
typedef std::list<LinePoint> LinePointGroup;

static double slope_to_angle(double slope) {
    return atan(slope) * (180.0 / M_PI);
}

// static double angle_to_slope(double angle) {
//     return tan(angle * M_PI / 180.0);
// }

static double calculate_angle(point_t point1, point_t point2)
{
    if (point2.x == point1.x) {
        return 90;
    }

    double k = (double)(point2.y - point1.y) / (point2.x - point1.x);
    auto angle = slope_to_angle(k);
    return angle;
}

static double calculate_slope(point_t point1, point_t point2)
{
    if (point2.x == point1.x) {
        return 10000;
    }
    return (point2.y - point1.y) / (point2.x - point1.x);
}

static std::vector<int> get_center_point(std::vector<std::vector<int>> points)
{
    std::vector<int> out;
    auto points_size = points.size();
    if (points_size) {
        int total_x = 0, total_y = 0;
        for (auto &point: points) {
            total_x += point[0];
            total_y += point[1];
        }

        total_x /= points_size;
        total_y /= points_size;

        out = {total_x, total_y};
    }

    return out;
}


static point_t get_center_point2(std::list<point_t> points)
{
    point_t out;
    auto points_size = points.size();
    if (points_size) {
        int total_x = 0, total_y = 0;
        for (auto &point: points) {
            total_x += point.x;
            total_y += point.y;
        }

        total_x /= points_size;
        total_y /= points_size;

        out = {total_x, total_y};
    }

    return out;
}

static std::vector<std::vector<int>> blobs_to_points(std::vector<std::vector<image::Blob>> blobs_vector)
{
    std::vector<std::vector<int>> output;
    for (auto &blobs : blobs_vector) {
        for (auto &b : blobs) {
            auto mini_corners = b.mini_corners();
            auto center_pointer = get_center_point(mini_corners);
            if (!center_pointer.empty()) {
                output.push_back(center_pointer);
            }
        }
    }

    return output;
}

static int get_points_distance(std::vector<int> point1, std::vector<int> point2)
{
    return std::sqrt(std::pow(point2[0] - point1[0], 2) + std::pow(point2[1] - point1[1], 2));
}

static int get_points_distance2(point_t point1, point_t point2)
{
    return std::sqrt(std::pow(point2.x - point1.x, 2) + std::pow(point2.y - point1.y, 2));
}

static std::vector<std::vector<int>> merge_points(std::vector<std::vector<int>> points, int distance)
{
    DEBUG_EN(0);
    DEBUG_PRT("max distance of merge:%d", distance);
    std::vector<std::vector<int>> new_points;

    for (size_t i = 0; i < points.size(); i ++) {
        std::vector<std::vector<int>> temp_points;
        DEBUG_PRT("check points size:%ld", points.size());
        for (size_t j = 0; j < points.size(); j ++) {
            if (i == j) continue;
            if (points[i][0] < 0 || points[i][1] < 0 || points[j][0] < 0 || points[j][1] < 0) continue;

            auto new_distance = get_points_distance(points[i], points[j]);
            DEBUG_PRT("(%ld,%ld) p1(%d,%d) p2(%d,%d) distance:%d", i, j, points[i][0], points[i][1], points[j][0], points[j][1], new_distance);
            if (distance >= new_distance) {
                temp_points.push_back({points[i][0], points[i][1]});
                temp_points.push_back({points[j][0], points[j][1]});
                points[j] = {-1, -1};
            }
        }

        DEBUG_PRT("temp points size:%ld", temp_points.size());
        auto center_point = get_center_point(temp_points);
        if (!center_point.empty()) {
            new_points.push_back(center_point);
        }
    }

    for (size_t i = 0; i < points.size(); i ++) {
        if (points[i][0] < 0 || points[i][1] < 0) continue;
        new_points.push_back(points[i]);
    }

    return new_points;
}


static point_t found_the_first_point(std::list<point_t> &points_list)
{
    auto min_point = points_list.front();
    auto min_point_it = points_list.begin();
    for (auto it = points_list.begin(); it != points_list.end(); ++ it) {
        auto item = *it;
        if (item.y == min_point.y) {
            if (item.x >= min_point.x) {
                min_point = item;
                min_point_it = it;
            }
        } else if (item.y >= min_point.y) {
            min_point = item;
            min_point_it = it;
        }
    }
    points_list.erase(min_point_it);
    return min_point;
}

__attribute__((unused)) static void print_points_list(std::string msg, std::list<point_t> points)
{
    DEBUG_EN(0);
#if __DEBUG
    if (DEBUG_IS_ENABLE()) {
        printf("%s(%ld):\r\n", msg.c_str(), points.size());
        int i = 0;
        for (auto &point: points) {
            printf("p[%2d](%3d,%3d) ", i, point.x, point.y);
            i ++;
            if (i % 5 == 0) {
                printf("\r\n");
            }
        }
        printf("\r\n");
    }
#endif
}

static LinePoint sort_line_points_list(point_t first_point, LinePoint input)
{
    input.sort([first_point](point_t a, point_t b) {
        auto distance_a = get_points_distance2(a, first_point);
        auto distance_b = get_points_distance2(b, first_point);
        if (distance_a <= distance_b) {
            return true;
        } else {
            return false;
        }
    });

    return input;
}

static LinePointGroup search_new_lines_group(std::list<point_t> &point_sets, std::list<point_t> bifurcation_points, int p2p_max_distance, int p2l_max_distance, double max_angle, bool has_first_print, point_t first_point = {0, 0})
{
    DEBUG_EN(0);
    DEBUG_PRT(" =================== search new lines ================= max distance:%d", p2p_max_distance);
    LinePointGroup new_group;
    if (bifurcation_points.empty()) {
        DEBUG_PRT("bifurcation sets is empty");
        return new_group;
    }

    if (point_sets.empty()) {
        DEBUG_PRT("point sets is empty");
        for (auto it = bifurcation_points.begin(); it != bifurcation_points.end(); ++ it) {
            auto next_point = *it;

            LinePoint new_line;
            if (has_first_print) {
                new_line.push_back(first_point);
            }
            new_line.push_back(next_point);
            new_group.push_back(new_line);
            it = bifurcation_points.erase(it);
        }

        return new_group;
    }

    for (auto it = bifurcation_points.begin(); it != bifurcation_points.end(); ++ it) {
        auto next_point = *it;
        LinePoint new_line;
        std::list<point_t> bifurcation_points;
        LinePoint temp_points;

        // insert the first point
        if (has_first_print) {
            new_line.push_back(first_point);
        }
        new_line.push_back(next_point);

        while (1) {
            DEBUG_PRT("found next point, current point(%d,%d) new_line size:%ld point sets size:%ld", next_point.x, next_point.y, new_line.size(), point_sets.size());
            // find next point
            bifurcation_points.clear();
            temp_points.clear();
            print_points_list("point sets", point_sets);
            for (auto it = point_sets.begin(); it != point_sets.end(); ++ it) {
                auto item = *it;
                auto new_distance = get_points_distance2(item, next_point);
                // DEBUG_PRT("find point(%d,%d) distance:%d", item.x, item.y, new_distance);
                if (p2p_max_distance >= new_distance) {
                    if (new_line.check_point_is_valid(item, max_angle, p2l_max_distance)) {
                        DEBUG_PRT("search point(%d, %d) is [VALID]", item.x, item.y);
                        temp_points.push_back(item);
                        point_sets.erase(it);
                    } else {
                        DEBUG_PRT("search point(%d, %d) is [INVALID]", item.x, item.y);
                        bifurcation_points.push_back(item);
                        point_sets.erase(it);
                    }
                }
            }

            // push new point
            if (!temp_points.empty()) {
                auto first_point = next_point;
                sort_line_points_list(first_point, temp_points);
                for (auto &point: temp_points) {
                    new_line.push_back(point);
                }
                next_point = temp_points.front();

                for (auto &point: bifurcation_points) {
                    point_sets.push_back(point);
                }
            } else {
                if (!bifurcation_points.empty()) {
                    DEBUG_PRT(" =================== search new bifurcation lines =================");
                    DEBUG_PRT("bifurcation_points size:%ld", bifurcation_points.size());
                    DEBUG_PRT("bifurcation points to line");
                    LinePointGroup point_group;
                    std::list<point_t> current_point;
                    current_point.push_back(next_point);
                    auto check_group = search_new_lines_group(bifurcation_points, current_point, p2p_max_distance, p2l_max_distance, max_angle, false);
                    DEBUG_PRT("check bifurcation group size %ld", check_group.size());
                    for (auto &l: check_group) {
                        l.pop_front();
                        auto it = l.begin();
                        if (it != l.end()) {
                            auto item = *it;
                            bifurcation_points.push_back(item);
                            l.erase(it);
                        }

                        for (auto &p: l) {
                            point_sets.push_back(p);
                        }
                    }
#if __DEBUG
                    print_points_list("bifuration_points", bifurcation_points);
#endif

                    // find others line
                    auto group = search_new_lines_group(point_sets, bifurcation_points, p2p_max_distance, p2l_max_distance, max_angle, true, next_point);
                    DEBUG_PRT("found group size:%ld", group.size());
                    DEBUG_PRT("insert bifurcation lines");
                    for (auto &line: group) {
                        new_group.push_back(line);
                        // new_line.add_bifurcation_line(line);
                    }
                } else {
                    break;
                }
            }
        }
        new_group.push_back(new_line);

        it = bifurcation_points.erase(it);
    }

    return new_group;
}

static std::vector<LinePointGroup> search_lines_group(std::vector<std::vector<int>> points, int p2p_distance, int p2l_distance, double angle)
{
    DEBUG_EN(0);
    std::vector<LinePointGroup> new_groups;
    std::list<std::list<point_t>> new_lines;
    std::list<point_t> points_list;
    for (auto &point: points) {
        points_list.push_back({point[0], point[1]});
    }

    while (points_list.size() > 0) {
        // found first point
        auto first_point = found_the_first_point(points_list);
        DEBUG_PRT("check a new line, first point(%d,%d)", first_point.x, first_point.y);

        // found points
        std::list<point_t> current_point;
        current_point.push_back(first_point);
        auto new_group = search_new_lines_group(points_list, current_point, p2p_distance, p2l_distance, angle, false);
        if (!new_group.empty()) {
            new_groups.push_back(new_group);
        }
#if __DEBUG
        if (DEBUG_IS_ENABLE()) {
            for (auto &line: new_group) {
                print_points_list("group list", line);
            }
        }
#endif
    }

    return new_groups;
}

static void calculate_magnitude_theta_and_rho(point_t p1, point_t p2, int &magnitude, int &theta, int &rho)
{
    DEBUG_EN(0);
    auto angle = calculate_angle(p1, p2);
    DEBUG_PRT("angle %f", angle);
    magnitude = get_points_distance2(p1, p2);
    theta = 90 - angle;
    if (theta < 0) theta += 180;
    rho = fabs((p2.y - p1.y) * p1.x - (p2.x - p1.x) * p1.y) / sqrt(pow(p2.y - p1.y, 2) + pow(p2.x - p1.x, 2));
    DEBUG_PRT("magnitude %d theta:%d rho:%d", magnitude, theta, rho);
}

static image::Line points_to_line(std::list<point_t> points)
{
    DEBUG_EN(0);

    auto first_point = points.front();
    auto center_point = get_center_point2(points);
    auto back_point = points.back();
    DEBUG_PRT("Get first point(%d,%d) center point(%d,%d) back point(%d,%d)",
        first_point.x, first_point.y, center_point.x, center_point.y, back_point.x, back_point.y);
    point_t second_point;
    if (first_point.x == center_point.x) {
        second_point.x = first_point.x;
        second_point.y = back_point.y;
    } else {
        auto k = calculate_slope(first_point, center_point);
        auto b = first_point.y - first_point.x * k;
        second_point.y = back_point.y;
        second_point.x = (second_point.y - b) / k;
    }
    DEBUG_PRT("Calculate line points front(%d,%d) back(%d,%d)", first_point.x, first_point.y, back_point.x, back_point.y);

    int magnitude = 0, theta = 0, rho = 0;
    calculate_magnitude_theta_and_rho(first_point, back_point, magnitude, theta, rho);
    return image::Line(first_point.x, first_point.y, back_point.x, back_point.y, magnitude, theta, rho);
}


typedef struct {
    image::Line line;
    LinePoint points;
} temp_line_t;

static std::vector<temp_line_t>  sort_image_line_and_get_type(std::vector<temp_line_t> temp_lines, LineType &type)
{
    DEBUG_EN(0);
    std::sort(temp_lines.begin(), temp_lines.end(), [](temp_line_t line1, temp_line_t line2) {
        return line1.line.magnitude() < line2.line.magnitude();
    });

    #define CHECK_IS_PERPENDICULAR(theta_diff)  (theta_diff >= 75 && theta_diff <= 105)
    #define CHECK_IS_PARALLEL(theta_diff)       (theta_diff >= -15 && theta_diff <= 15)

    DEBUG_PRT("calculate the type of lines(%ld)", temp_lines.size());
    switch (temp_lines.size()) {
    case 2:
    {
        auto l1 = temp_lines[0].line;
        auto l2 = temp_lines[1].line;
        auto theta1 = l1.theta();
        auto theta2 = l2.theta();
        auto theta_diff = abs(theta2 - theta1);
        if (CHECK_IS_PERPENDICULAR(theta_diff)) {
            type = LineType::LINE_L;
        } else {
            type = LineType::LINE_NORMAL;
        }
        DEBUG_PRT("theta1:%d theta2:%d theta diff:%d type:%d", theta1, theta2, theta_diff, (int)type);
        break;
    }
    case 3:
    {
        auto l1 = temp_lines[0].line;
        auto l2 = temp_lines[1].line;
        auto l3 = temp_lines[2].line;
        auto theta1 = l1.theta();
        auto theta2 = l2.theta();
        auto theta3 = l3.theta();
        auto theta12_diff = abs(theta1 - theta2);
        // auto theta23_diff = abs(theta2 - theta3);
        auto theta13_diff = abs(theta1 - theta3);

        if (CHECK_IS_PERPENDICULAR(theta12_diff)) {
            if (CHECK_IS_PARALLEL(theta13_diff)) {
                type = LineType::LINE_T;
            } else {
                type = LineType::LINE_NORMAL;
            }
        } else if (CHECK_IS_PERPENDICULAR(theta13_diff)) {
            if (CHECK_IS_PARALLEL(theta12_diff)) {
                type = LineType::LINE_T;
            } else {
                type = LineType::LINE_NORMAL;
            }
        } else {
            type = LineType::LINE_NORMAL;
        }

        DEBUG_PRT("theta12_diff:%d theta23_diff:%d theta13_diff:%d type:%d", theta12_diff, theta23_diff, theta13_diff, (int)type);
        break;
    }
    case 4:
    {
        auto l1 = temp_lines[0].line;
        auto l2 = temp_lines[1].line;
        auto l3 = temp_lines[2].line;
        auto l4 = temp_lines[2].line;
        auto theta1 = l1.theta();
        auto theta2 = l2.theta();
        auto theta3 = l3.theta();
        auto theta4 = l4.theta();
        auto theta12_diff = abs(theta1 - theta2);
        auto theta13_diff = abs(theta1 - theta3);
        auto theta14_diff = abs(theta1 - theta4);
        // auto theta23_diff = abs(theta2 - theta3);
        auto theta24_diff = abs(theta2 - theta4);
        // auto theta34_diff = abs(theta3 - theta4);

        if (CHECK_IS_PERPENDICULAR(theta12_diff) && CHECK_IS_PERPENDICULAR(theta14_diff) && CHECK_IS_PARALLEL(theta13_diff) && CHECK_IS_PARALLEL(theta24_diff)) {
            type = LineType::LINE_CROSS;
        } else {
            type = LineType::LINE_NORMAL;
        }

        DEBUG_PRT("theta12_diff:%d theta23_diff:%d theta13_diff:%d type:%d", theta12_diff, theta23_diff, theta13_diff, (int)type);
        break;
    }
    default:
        type = LineType::LINE_NORMAL;
        DEBUG_PRT("A normal type:%d", (int)type);
        break;
    }

    return temp_lines;
}

std::vector<image::LineGroup> Image::search_line_path(std::vector<std::vector<int>> thresholds, int detect_pixel_size, int point_merge_size, int connection_max_size, int connection_max_distance, int connection_max_angle)
{
    DEBUG_EN(0);
    std::vector<LineGroup> line_group;
    auto gray_img = (image::Image *)nullptr;
    auto need_free_gray_img = false;
    if (format() != image::FMT_GRAYSCALE) {
        gray_img = this->to_format(image::FMT_GRAYSCALE);
        need_free_gray_img = true;
    } else {
        gray_img = this;
        need_free_gray_img = false;
    }

#if __DEBUG
    uint64_t t = time::ticks_ms(), t2 = 0;
#endif
    auto gray_img_width = gray_img->width();
    auto gray_img_height = gray_img->height();

    // found blobs
    std::vector<std::vector<image::Blob>> result_of_find_blobs;
    int x_stride = 2;
    int y_stride = 1;
    int area_threshold = 10;
    int pixels_threshold = detect_pixel_size * detect_pixel_size / 4;
    for (int h = 0; h < gray_img_height; h += detect_pixel_size) {
        for (int w = 0; w < gray_img_width; w += detect_pixel_size) {
            std::vector<int> roi = {w, h, detect_pixel_size, detect_pixel_size};
            auto blobs = gray_img->find_blobs(thresholds, false, roi, x_stride, y_stride, area_threshold, pixels_threshold);
            result_of_find_blobs.push_back(blobs);
        }
    }
#if __DEBUG
    t2 = time::ticks_ms(), log::info("find blobs use %lld ms", t2 - t), t = time::ticks_ms();
#endif
    // collect points
    auto blobs_to_points_res = blobs_to_points(result_of_find_blobs);
#if _DEBUG
    t2 = time::ticks_ms(), log::info("collect points use %lld ms", t2 - t);
#endif
#if 0
    for (auto &p: blobs_to_points_res) {
        this->draw_cross(p[0], p[1], image::COLOR_RED, 5, 2);
    }
#endif
#if __DEBUG
    if (DEBUG_IS_ENABLE()) {
        DEBUG_PRT("blobs_to_points_res size:%ld", blobs_to_points_res.size());
    }
#endif
#if __DEBUG
    t = time::ticks_ms();
#endif
    // merge points
    auto merge_points_res = merge_points(blobs_to_points_res, point_merge_size);
#if __DEBUG
    t2 = time::ticks_ms(), log::info("merge points use %lld ms", t2 - t);
#endif

#if 1
    for (auto &p: merge_points_res) {
        this->draw_cross(p[0], p[1], image::COLOR_GRAY, 5, 2);
    }
#endif
#if __DEBUG
    if (DEBUG_IS_ENABLE()) {
        DEBUG_PRT("merge_points_res size:%ld", merge_points_res.size());
    }
#endif
#if __DEBUG
    t = time::ticks_ms();
#endif
    // search lines group
    auto search_lines_group_res = search_lines_group(merge_points_res, connection_max_size, connection_max_distance, connection_max_angle);
#if __DEBUG
    t2 = time::ticks_ms(), log::info("search lines group use %lld ms", t2 - t), t = time::ticks_ms();
#endif

#if __DEBUG
    if (DEBUG_IS_ENABLE()) {
        auto g_count = 0, l_count = 0;
        DEBUG_PRT("search_lines_group_res size:%ld", search_lines_group_res.size());
        for (auto &group: search_lines_group_res) {
            DEBUG_PRT("group[%d] size:%ld", g_count, group.size());
            for (auto &line: group) {
                DEBUG_PRT("group[%d] line[%d] size:%ld", g_count, l_count ++, line.size());
            }
            g_count ++;
        }
    }
#endif
#if __DEBUG
    t = time::ticks_ms();
#endif
    int group_id = 0;
    for (auto &group: search_lines_group_res) {
        std::vector<temp_line_t> temp_lines;

        auto type = image::LineType::LINE_NORMAL;

        bool group_is_valid = false;
        for (auto &points: group) {
#if __DEBUG
            if (DEBUG_IS_ENABLE()) {
                print_points_list("print points", points);
            }
#endif
            if (points.size() >= 3) {
                points = sort_line_points_list(points.front(), points);
                auto new_line = points_to_line(points);
                temp_lines.push_back({new_line, points});
                group_is_valid = true;
            }
        }

        if (group_is_valid) {
            std::vector<image::Line> new_lines;
            std::vector<std::vector<std::vector<int>>> new_points;
            temp_lines = sort_image_line_and_get_type(temp_lines, type);
            for (auto &temp_line: temp_lines) {
                std::vector<std::vector<int>> points;
                for (auto &p: temp_line.points) {
                    points.push_back({p.x, p.y});
                }
                new_points.push_back(points);
                new_lines.push_back(temp_line.line);
            }
            LineGroup new_group(group_id ++, type, new_lines, new_points);
            line_group.push_back(new_group);
        }
    }
#if __DEBUG
    t2 = time::ticks_ms(), log::info("create result use %lld ms", t2 - t), t = time::ticks_ms();
#endif
    if (need_free_gray_img) {
        delete gray_img;
    }
    return line_group;
}
}
