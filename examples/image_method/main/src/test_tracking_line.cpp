#include "test_image.hpp"
#include "opencv2/opencv.hpp"
#include <list>

#define USE_OPENCV      (0)
#define USE_OPENCV_2    (0)

#if USE_OPENCV

typedef struct {
    int x, y;
} point_t;
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
        // Ax + By + C = 0
        A = p2.y - p1.y;
        B = p1.x - p2.x;
        C = p2.x * p1.y - p1.x * p2.y;

        rho = fabs(C) / sqrt(A * A + B * B);
        if (B == 0) {
            theta = M_PI / 2;
        } else {
            theta = atan2(-A, B);
        }

        if (theta < 0) {
            theta += M_PI;
        }
    }

    NewLine() {

    }
    NewLine(cv::Point &start_point, cv::Point &end_point) {
        start = start_point;
        end = end_point;
        center = cv::Point(start.x + (end.x - start.x) / 2, start.y + (end.y - start.y) / 2);
        calculate_magnitude_theta_and_rho({start.x, start.y}, {end.x, end.y}, A, B, C, magnitude, theta, rho);
    }

    double degree() {
        auto degree = theta * 180.0 / M_PI;
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
    bool compute_y(int x, int &y) {
        float sin_theta = std::sin(theta);
        float cos_theta = std::cos(theta);
        if (std::abs(sin_theta) < 1e-6) {
            return false;
        }

        y = (rho - x * cos_theta) / sin_theta;
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
        start.x = (start.x + line.start.x) / 2;
        start.y = (start.y + line.start.y) / 2;
        end.x = (end.x + line.end.x) / 2;
        end.y = (end.y + line.end.y) / 2;
        center = cv::Point(center.x + (end.x - start.x) / 2, start.y + (end.y - start.y) / 2);
        calculate_magnitude_theta_and_rho({start.x, start.y}, {end.x, end.y}, A, B, C, magnitude, theta, rho);
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

    image::LineType get_lines_type(NewLine &line, int min_diff_of_degree = 70, int max_point_to_line_distance = 50) {
        if (abs(line.degree() - degree()) >= min_diff_of_degree) {
            auto distance1 = point_distance(line.start.x, line.start.y);
            auto distance2 = point_distance(line.end.x, line.end.y);
            auto distance3 = line.point_distance(start.x, start.y);
            auto distance4 = line.point_distance(end.x, end.y);
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

static void sort_vertical_point(cv::Point start, cv::Point end, cv::Point &out_start, cv::Point &out_end) {
    if (start.y >= end.y) {
        out_start = start;
        out_end = end;
    } else {
        out_start = end;
        out_end = start;
    }
}

std::vector<image::LineGroup> detectCrossAndT(image::Image *img, int merge_degree = 10) {
    auto gray_img = (image::Image *)nullptr;
    auto need_free_gray_img = false;
    if (img->format() != image::FMT_GRAYSCALE) {
        gray_img = img->to_format(image::FMT_GRAYSCALE);
        need_free_gray_img = true;
    } else {
        gray_img = img;
        need_free_gray_img = false;
    }

    cv::Mat edges;
    cv::Mat gray = cv::Mat(gray_img->height(), gray_img->width(), CV_8UC((int)image::fmt_size[img->format()]), img->data());

    // cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
    cv::Canny(gray, edges, 50, 150);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, 1, CV_PI / 180, 20, 50, 100);

    std::list<NewLine> new_lines;
    std::list<NewLine> merged_lines;

    for (const auto& l : lines) {
        cv::Point start(l[0], l[1]);
        cv::Point end(l[2], l[3]);
        new_lines.push_back(NewLine(start, end));
    }

    log::info(" ========== HoughLinesP found lines size:%d ========== ", new_lines.size());
    for (auto it = new_lines.begin(); it != new_lines.end(); it ++) {
        auto line = *it;
        cv::line(gray, line.start, line.end, cv::Scalar(128, 0, 0), 1);
    }

    while (!new_lines.empty()) {
        auto it1 = new_lines.begin();
        if (it1 == new_lines.end()) {
            break;
        }
        auto line1 = *it1;
        for (auto it2 = std::next(it1); it2 != new_lines.end();) {
            auto line2 = *it2;//log::info(" line1 degree:%f, line2 degree:%f diff degree:%f", line1.degree(), line2.degree(), fabs(line1.degree() - line2.degree()));
            if (fabs(line1.degree() - line2.degree()) <= merge_degree) {
                line1.merge_line(line2);
                it2 = new_lines.erase(it2);
            } else {
                it2 ++;
            }
        }

        merged_lines.push_back(line1);
        new_lines.erase(it1);
    }

    log::info(" ========== Merged lines size:%d ========== ", merged_lines.size());
    std::vector<image::LineGroup> groups;
    auto group_type = image::LineType::LINE_NORMAL;
    int group_idx = 0;
    if (merged_lines.size() == 2) {
        auto front_line = merged_lines.front();
        auto back_line = merged_lines.back();
        group_idx = 0;
        group_type = front_line.get_lines_type(back_line);
        std::vector<image::Line> lines;

        switch (group_type) {
        case image::LineType::LINE_L:
        {
            NewLine ver_line, hor_line;
            // Check the straight line towards the bottom of the screen
            if (front_line.center.y >= back_line.center.y) {
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
            if (front_line.center.y >= back_line.center.y) {
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
            if (ver_line.degree() >= 90) {
                if (ver_line.point_distance_without_abs(hor_line.start.x, hor_line.start.y) > 0) {
                    end = {hor_line.start.x, hor_line.start.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                    end = {hor_line.end.x, hor_line.end.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                } else {
                    end = {hor_line.end.x, hor_line.end.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                    end = {hor_line.start.x, hor_line.start.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                }
            } else {
                if (ver_line.point_distance_without_abs(hor_line.start.x, hor_line.start.y) > 0) {
                    end = {hor_line.end.x, hor_line.end.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                    end = {hor_line.start.x, hor_line.start.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                } else {
                    end = {hor_line.start.x, hor_line.start.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                    end = {hor_line.end.x, hor_line.end.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                }
            }
        }
        break;
        case image::LineType::LINE_CROSS:
        {
            NewLine ver_line, hor_line;
            // Check the straight line towards the bottom of the screen
            if (front_line.center.y >= back_line.center.y) {
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

            // push horizontal line
            start = {intersection_point.x, intersection_point.y};
            if (ver_line.degree() >= 90) {
                if (ver_line.point_distance_without_abs(hor_line.start.x, hor_line.start.y) > 0) {
                    end = {hor_line.start.x, hor_line.start.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                    end = {hor_line.end.x, hor_line.end.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                } else {
                    end = {hor_line.end.x, hor_line.end.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                    end = {hor_line.start.x, hor_line.start.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                }
            } else {
                if (ver_line.point_distance_without_abs(hor_line.start.x, hor_line.start.y) > 0) {
                    end = {hor_line.end.x, hor_line.end.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                    end = {hor_line.start.x, hor_line.start.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                } else {
                    end = {hor_line.start.x, hor_line.start.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                    end = {hor_line.end.x, hor_line.end.y};
                    lines.push_back(image::Line(start.x, start.y, end.x, end.y, hor_line.magnitude, hor_line.theta, hor_line.rho));
                }
            }
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

    if (need_free_gray_img) {
        delete gray_img;
    }

    return groups;
}

static image::Color get_color()
{
    static int color_count = 0;
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
static std::string get_lines_type(image::LineType type)
{
    switch (type) {
    case image::LineType::LINE_NORMAL:
        return "normal";
    case image::LineType::LINE_CROSS:
        return "cross";
    case image::LineType::LINE_L:
        return "L";
    case image::LineType::LINE_T:
        return "T";
    default: return "Unknown";
    }
}

int test_tracking_line(image::Image *img) {
    uint64_t t = time::ticks_ms(), t2 = 0;
    auto groups = detectCrossAndT(img);
    t2 = time::ticks_ms(), log::info("search line path use %lld ms", t2 - t);
    log::info("groups size:%d", groups.size());
    // if (groups.size() > 2) {
    //     static int save_count = 0;
    //     std::string save_path = "/root/test" + std::to_string(save_count++) + ".jpg";
    //     img->save(save_path.c_str());
    // }
    for (auto &group: groups) {
        auto lines = group.lines();
        auto id = group.id();
        log::info("group lines size:%d", lines.size());
        auto type_str = get_lines_type(group.type());
        auto color = get_color();
        for (size_t i = 0; i < lines.size(); i ++) {
            auto l = lines[i];
            img->draw_line(l.x1(), l.y1(), l.x2(), l.y2(), color, 2);
            img->draw_string(abs(l.x1() + l.x2()) / 2, abs(l.y1() + l.y2()) / 2, std::to_string(id) + " " + std::to_string(i) + type_str, color);
        }
    }
    return 0;
}
#elif USE_OPENCV_2
#include <opencv2/opencv.hpp>

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
static std::vector<cv::Point2f> blobs_to_points(std::vector<std::vector<image::Blob>> blobs_vector)
{
    std::vector<cv::Point2f> output;
    for (auto &blobs : blobs_vector) {
        for (auto &b : blobs) {
            auto mini_corners = b.mini_corners();
            auto center_pointer = get_center_point(mini_corners);
            if (!center_pointer.empty()) {
                output.push_back(cv::Point2f(center_pointer[0], center_pointer[1]));
            }
        }
    }

    return output;
}

static int color_count = 0;
static void color_init() {color_count = 0;}
static image::Color get_color()
{
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

double pointToLineDistance(double x0, double y0, double rho, double theta) {
    // 计算直线的 A, B, C 参数
    double A = cos(theta);
    double B = sin(theta);
    double C = -rho;
log::info("A:%f B:%f C:%f x0:%f y0:%f", A, B, C, x0, y0);
    // 计算点到直线的距离
    double distance = fabs(A * x0 + B * y0 + C) / sqrt(A * A + B * B);
    log::info("pointToLineDistance:%f", distance);
    return distance;
}

class UserLine {
public:
    // A⋅x+B⋅y+C=0
    double A;
    double B;
    double C;
    double theta;
    double rho;
    UserLine(double rho, double theta) {
        this->rho = rho;
        this->theta = theta;
        this->A = cos(theta);
        this->B = sin(theta);
        this->C = -rho;
        log::info("A:%f B:%f C:%f", A, B, C);
    }

    int get_distance(int x, int y) {
        return fabs(A * x + B * y + C) / sqrt(A * A + B * B);
    }

    double get_distance(double x, double y) {
        return fabs(A * x + B * y + C) / sqrt(A * A + B * B);
    }

    int get_y(int x) {
        return (rho - x * A) / B;
    }
};

std::vector<image::LineGroup> search_line_path(image::Image *img, std::vector<std::vector<int>> thresholds, int detect_pixel_size, int point_merge_size, int connection_max_size, int connection_max_distance, int connection_max_angle)
{
    std::vector<image::LineGroup> line_group;
    auto gray_img = (image::Image *)nullptr;
    auto need_free_gray_img = false;
    if (img->format() != image::FMT_GRAYSCALE) {
        gray_img = img->to_format(image::FMT_GRAYSCALE);
        need_free_gray_img = true;
    } else {
        gray_img = img;
        need_free_gray_img = false;
    }

    auto gray_img_width = gray_img->width();
    auto gray_img_height = gray_img->height();

    // found blobs
    std::vector<std::vector<image::Blob>> result_of_find_blobs;
    int x_stride = 2;
    int y_stride = 1;
    int area_threshold = 5;
    int pixels_threshold = detect_pixel_size * detect_pixel_size / 4;
    for (int h = 0; h < gray_img_height; h += detect_pixel_size) {
        for (int w = 0; w < gray_img_width; w += detect_pixel_size) {
            std::vector<int> roi = {w, h, detect_pixel_size, detect_pixel_size};
            auto blobs = gray_img->find_blobs(thresholds, false, roi, x_stride, y_stride, area_threshold, pixels_threshold);
            result_of_find_blobs.push_back(blobs);
        }
    }

    // collect points
    auto blobs_to_points_res = blobs_to_points(result_of_find_blobs);


    double rhoMin = 0.0f, rhoMax = std::sqrt(gray_img_width * gray_img_width + gray_img_height * gray_img_height), rhoStep = 1;
    double thetaMin = 0.0f, thetaMax = CV_PI / 2.0f, thetaStep = CV_PI / 180.0f;
    while (!blobs_to_points_res.empty()) {

        log::info("blobs_to_points_res size:%d", blobs_to_points_res.size());
        auto color = get_color();
        for (auto &p: blobs_to_points_res) {
            img->draw_cross(p.x, p.y, color, 5, 1);
        }

        cv::Mat lines;
        std::vector<cv::Vec3d> line3d;
        uint64_t t = time::ticks_ms(), t2 = 0;
        HoughLinesPointSet(blobs_to_points_res, lines, 1, 1,
                        rhoMin, rhoMax, rhoStep,
                        thetaMin, thetaMax, thetaStep);


        line3d.clear();
        for (int i = 0; i < lines.rows; ++i) {
            line3d.emplace_back(lines.at<cv::Vec3d>(i, 0));
        }

        if (line3d.empty()) {
            break;
        }

        for (size_t i = 0; i < line3d.size(); i++) {
            // log::info("[%d] line3d votes:%f rho:%f theta:%f", i, line3d.at(i).val[0], line3d.at(i).val[1], line3d.at(i).val[2]);

            double rho = line3d.at(i).val[1];
            double theta = line3d.at(i).val[2];

            UserLine line(rho, theta);
            double a = cos(theta), b = sin(theta);
            double x1 = 0, x2 = img->width();
            double y1 = line.get_y(x1), y2 = line.get_y(x2);
            // img->draw_line(x1, y1, x2, y2, get_color());
            // img->draw_line(x1, y1, x2, y2, color);
            img->draw_line(x1, y1, x2, y2, image::COLOR_RED);

            for (auto it = blobs_to_points_res.begin(); it != blobs_to_points_res.end();) {
                auto point = *it;
                auto distance = line.get_distance(point.x, point.y);
                // log::info("point(%f,%f) distance:%f", point.x, point.y, distance);
                if (distance < 10) {
                    it = blobs_to_points_res.erase(it);
                } else {
                    it ++;
                }
            }
        }

        t2 = time::ticks_ms(), log::info("HoughLinesPointSet use %lld ms", t2 - t);
    }

    log::info("blobs_to_points_res size:%d", blobs_to_points_res.size());
    auto color = get_color();
    for (auto &p: blobs_to_points_res) {
        img->draw_cross(p.x, p.y, color, 5, 1);
    }

    if (need_free_gray_img) {
        delete gray_img;
    }
    return line_group;
}

int test_tracking_line(image::Image *img) {
    uint64_t t = time::ticks_ms(), t2 = 0;
    auto thresholds = (std::vector<std::vector<int>>){{0, 50, -128, 127, -128, 127}};
    auto detect_pixel_size = 15;
    auto point_merge_size = 10;
    auto connection_max_size = 30;
    auto connection_max_distance = 30;
    auto connection_max_angle = 10;
    search_line_path(img, thresholds, detect_pixel_size, point_merge_size, connection_max_size, connection_max_distance, connection_max_angle);
    t2 = time::ticks_ms(), log::info("search line path use %lld ms", t2 - t);
    return 0;
}

#else
static int color_count = 0;
static void reset_color_count() {
    color_count = 0;
}

static image::Color get_color()
{
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

static std::string get_lines_type(image::LineType type)
{
    switch (type) {
    case image::LineType::LINE_NORMAL:
        return "normal";
    case image::LineType::LINE_CROSS:
        return "cross";
    case image::LineType::LINE_L:
        return "L";
    case image::LineType::LINE_T:
        return "T";
    default: return "Unknown";
    }
}
int test_tracking_line(image::Image *img) {
    uint64_t t = time::ticks_ms(), t2 = 0;
    auto threshold = 30;
    auto merge_degree = 20;
    auto group_list = img->search_line_path(threshold, merge_degree);
    t2 = time::ticks_ms(), log::info("search line path use %lld ms, group size:%d", t2 - t, group_list.size());
    reset_color_count();
    for (auto &group: group_list) {
        auto lines = group.lines();
        auto type_str = get_lines_type(group.type());
        for (size_t i = 0; i < lines.size(); i ++) {
            auto l = lines[i];
            auto color = get_color();

            img->draw_line(l.x1(), l.y1(), l.x2(), l.y2(), color, 2);
            img->draw_string(abs(l.x1() + l.x2()) / 2, abs(l.y1() + l.y2()) / 2, type_str + " " + std::to_string(i), color);
        }
    }
    return 0;
}
#endif

