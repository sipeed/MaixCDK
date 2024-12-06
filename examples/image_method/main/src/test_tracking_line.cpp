#include "test_image.hpp"

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

static std::string get_line_type(image::LineType type)
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
    auto thresholds = (std::vector<std::vector<int>>){{0, 50, -128, 127, -128, 127}};
    auto detect_pixel_size = 15;
    auto point_merge_size = 20;
    auto connection_max_size = 50;
    auto connection_max_distance = 30;
    auto connection_max_angle = 20;
    auto group_list = img->search_line_path(thresholds,
    detect_pixel_size, point_merge_size, connection_max_size, connection_max_distance, connection_max_angle);
    t2 = time::ticks_ms(), log::info("search line path use %lld ms, group size:%d", t2 - t, group_list.size());

    color_init();
    for (auto &group: group_list) {
        auto lines = group.lines();
        auto points = group.points();
        auto id = group.id();
        auto type_str = get_line_type(group.type());
        for (size_t i = 0; i < lines.size(); i ++) {
            auto point = points[i];
            auto l = lines[i];
            auto color = get_color();

            for (auto &p: point) {
                img->draw_cross(p[0], p[1], color, 2, 3);
            }

            img->draw_line(l.x1(), l.y1(), l.x2(), l.y2(), color, 2);
            img->draw_string(abs(l.x1() + l.x2()) / 2, abs(l.y1() + l.y2()) / 2, std::to_string(id) + " " + type_str, color);
        }
    }
    return 0;
}