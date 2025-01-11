#include "focus.hpp"

#include <cmath>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "maix_basic.hpp" // maix::time maix::log

constexpr bool AUTO_FOCUS_DEBUG = 0;

#define dbg(fmt, ...) if constexpr (AUTO_FOCUS_DEBUG) { maix::log::info(fmt, ##__VA_ARGS__); }


namespace MAIX_FOCUS_NAMESPACE {

/*****************************************************************************/
/* AutoFocusStepInfo */
AutoFocusStepInfo::AutoFocusStepInfo(int lf) noexcept
{
    this->update(lf);
}

AutoFocusStepInfo::AutoFocusStepInfo(int sf, int sn) noexcept
{
    this->sf = sf;
    this->sn = sn;
    this->update(-1);
}

void AutoFocusStepInfo::update(int lf) noexcept
{
    if (lf > 0) {
        this->lf = std::abs(lf);
        this->lf -= this->lf % 2;

        float sf_float = std::pow(
            (static_cast<float>(this->lf) / 2), 2.0f / 3.0f);
        this->sf = static_cast<int>(std::round(sf_float));

        while (this->lf % this->sf != 0) {
            this->sf--;
        }

        this->sn = this->lf / this->sf;
        if (this->sn > this->sf) {
            std::swap(this->sn, this->sf);
        }
    }
    this->soft = (this->sf / this->sn) * this->sn;
}


/*****************************************************************************/
/* PointMapper */
PointMapper::PointMapper(int src_w, int src_h, int dst_w, int dst_h) noexcept
    : _sw{src_w}, _sh{src_h}, _dw{dst_w}, _dh{dst_h} {}

std::pair<int, int> PointMapper::map_s2d(int sx, int sy) noexcept
{
    int dx = static_cast<int>((static_cast<float>(sx) / this->_sw) * this->_dw);
    int dy = static_cast<int>((static_cast<float>(sy) / this->_sh) * this->_dh);
    return {dx, dy};
}

std::pair<int, int> PointMapper::map_d2s(int dx, int dy) noexcept
{
    int sx = static_cast<int>((static_cast<float>(dx) / this->_dw) * this->_sw);
    int sy = static_cast<int>((static_cast<float>(dy) / this->_dh) * this->_sh);
    return {sx, sy};
}


/*****************************************************************************/
/* CVMatCreater */
CVMatCreater::CVMatCreater() noexcept
    : _creater(std::make_unique<ImageForward>()) {}

CVMatCreater::CVMatCreater(Box<IImageCreater> img_creater)
        : _creater(std::move(img_creater))
{
    if (!this->_creater) {
        std::stringstream ss;
        ss << '[' << __PRETTY_FUNCTION__ << std::to_string(__LINE__) << ']'
            << " Obj::_creater is nullptr!";
        throw std::runtime_error(ss.str());
    }
}

CVMatCreater::RT CVMatCreater::get(ArcBox<Image> img)
{
    auto curr_img = this->_creater->create(img);
    cv::Mat mat(curr_img->height(), curr_img->width(), CV_8UC1, curr_img->data());
    return std::make_pair(mat, curr_img);
}


/*****************************************************************************/
/* ImageForward */
ArcBox<ImageForward::Image> ImageForward::create(ArcBox<Image> img)
{
    return img;
}


/*****************************************************************************/
/* ImageGrayCreater */
ImageGrayCreater::ImageGrayCreater() noexcept
    : _prev(std::make_unique<ImageForward>()) {}

ImageGrayCreater::ImageGrayCreater(Box<IImageCreater> prev_creater)
    : _prev(std::move(prev_creater))
{
    if (!this->_prev) {
        std::stringstream ss;
        ss << '[' << __PRETTY_FUNCTION__ << std::to_string(__LINE__) << ']'
            << " Obj::_creater is nullptr!";
        throw std::runtime_error(ss.str());
    }
}

ArcBox<ImageGrayCreater::Image> ImageGrayCreater::create(ArcBox<Image> img)
{
    auto prev_img = this->_prev->create(img);
    auto ptr = prev_img->to_format(maix::image::FMT_GRAYSCALE);
    return ArcBox<Image>(ptr);
}


/*****************************************************************************/
/* IImageCropper */
IImageCropper::Rect::Rect() noexcept
    : x{}, y{}, w{}, h{} {}

IImageCropper::Rect::Rect(int x, int y, int w, int h) noexcept
    : x{x}, y{y}, w{w}, h{h} {}

/* rect default impl */
IImageCropper::Rect IImageCropper::rect(int cx, int cy, int w, int h, int iw, int ih)
{
    Rect rect;

    // dbg("IImageCropper::rect %dx%d, (%d,%d), %dx%d", w, h, cx, cy, iw, ih);

    rect.w = std::min(w, iw);
    rect.h = std::min(h, ih);
    rect.x = std::max(0, cx - rect.w / 2);
    rect.y = std::max(0, cy - rect.h / 2);
    if (rect.x + rect.w > iw) {
        rect.x = std::max(iw - rect.w, 0);
    }
    if (rect.y + rect.h > ih) {
        rect.y = std::max(ih - rect.h, 0);
    }
    // dbg("IImageCropper::rect %dx%d, (%d,%d), %dx%d", rect.w, rect.h, rect.x, rect.y, iw, ih);


    return rect;
}

IImageCropper::Rect IImageCropper::rect(const Rect& cr, int iw, int ih)
{
    return this->rect(cr.x, cr.y, cr.w, cr.h, iw, ih);
}


/*****************************************************************************/
/* ImageCropper */
ImageCropper::ImageCropper(int cx, int cy, int w, int h) noexcept
    : _cr{cx, cy, w, h}, _prev(std::make_unique<ImageForward>()) {}

ImageCropper::ImageCropper(int cx, int cy, int w, int h, Box<IImageCreater> prev_creater)
    : _cr{cx, cy, w, h}, _prev(std::move(prev_creater))
{
    if (!this->_prev) {
        std::stringstream ss;
        ss << '[' << __PRETTY_FUNCTION__ << std::to_string(__LINE__) << ']'
            << " Obj::_creater is nullptr!";
        throw std::runtime_error(ss.str());
    }
}

/* use default rect impl */
ImageCropper::Rect ImageCropper::rect(int cx, int cy, int w, int h, int iw, int ih)
{
    return IImageCropper::rect(cx, cy, w, h, iw, ih);
}

ImageCropper::Rect ImageCropper::rect(const Rect& cr, int iw, int ih)
{
    return IImageCropper::rect(cr, iw, ih);
}

ArcBox<ImageCropper::Image> ImageCropper::create(ArcBox<Image> img)
{
    auto prev_img = this->_prev->create(img);
    auto rect = this->rect(this->_cr, img->width(), img->height());
    auto ptr = prev_img->crop(rect.x, rect.y, rect.w, rect.h);
    return ArcBox<Image>(ptr);
}


/*****************************************************************************/
/* ImageNV21Cropper */
ImageYVU420SPNV21Cropper::ImageYVU420SPNV21Cropper(int cx, int cy, int w, int h) noexcept
    : _cr{cx, cy, w, h}, _prev(std::make_unique<ImageForward>()) {}

ImageYVU420SPNV21Cropper::ImageYVU420SPNV21Cropper(int cx, int cy, int w, int h, Box<IImageCreater> prev_creater)
    : _cr{cx, cy, w, h}, _prev(std::move(prev_creater))
{
    if (!this->_prev) {
        std::stringstream ss;
        ss << '[' << __PRETTY_FUNCTION__ << std::to_string(__LINE__) << ']'
            << " Obj::_creater is nullptr!";
        throw std::runtime_error(ss.str());
    }
}

/* default impl */
ImageYVU420SPNV21Cropper::Rect ImageYVU420SPNV21Cropper::rect(int cx, int cy, int w, int h, int iw, int ih)
{
    return IImageCropper::rect(cx, cy, w, h, iw, ih);
}

ImageYVU420SPNV21Cropper::Rect ImageYVU420SPNV21Cropper::rect(const Rect& cr, int iw, int ih)
{
    return IImageCropper::rect(cr, iw, ih);
}

ArcBox<ImageYVU420SPNV21Cropper::Image> ImageYVU420SPNV21Cropper::create(ArcBox<Image> img)
{
    auto prev_img = this->_prev->create(img);

    if (prev_img->format() != image::FMT_YVU420SP) {
        maix::log::warn("[%s] format != FMT_YVU420SP, return input image");
        return img;
    }

    auto rect = this->rect(this->_cr, img->width(), img->height());

    rect.x += rect.x % 2;
    rect.y += rect.y % 2;
    rect.w -= rect.w % 2;
    rect.h -= rect.h % 2;

    /* crop */
    /* YYYY...VUVUVU...VUVU, NV21 */
    /* YYYY V U */
    const int pixels = rect.w * rect.h;
    ArcBox<Image> res = std::make_shared<Image>(rect.w, rect.h, image::FMT_YVU420SP);

    // get ptr
    const uint8_t* src_data = static_cast<uint8_t*>(prev_img->data());
    uint8_t* dst_data_y = static_cast<uint8_t*>(res->data()); // dst Y
    uint8_t* dst_data_uv = dst_data_y + pixels; // dst UV

    /* YYYYY... */
    for (int i = 0; i < rect.h; ++i) {
        for (int j = 0; j < rect.w; ++j) {
            int src_index = (rect.y + i) * prev_img->width() + (rect.x + j);
            dst_data_y[i * rect.w + j] = src_data[src_index];
        }
    }

    /* VUVUVU... */
    for (int i = 0; i < rect.h / 2; ++i) {
        for (int j = 0; j < rect.w / 2; ++j) {
            int src_index_uv = (rect.y / 2 + i) * (prev_img->width() / 2) + (rect.x / 2 + j);
            dst_data_uv[i * rect.w + j * 2] = src_data[prev_img->width() * prev_img->height() + src_index_uv * 2];     // V
            dst_data_uv[i * rect.w + j * 2 + 1] = src_data[prev_img->width() * prev_img->height() + src_index_uv * 2 + 1]; // U
        }
    }

    /* debug: check crop */
    // {
    //     static int cnt = 0;
    //     dbg("rect %dx%d (%d,%d)", rect.w, rect.h, rect.x, rect.y);
    //     Box<Image> save_img = Box<Image>(res->to_format(image::FMT_JPEG));
    //     std::stringstream ss;
    //     ss << "/root/ImageYVU420SPNV21Cropper_" << std::to_string(cnt) << ".jpg";
    //     save_img->save(ss.str().c_str());
    //     cnt++;
    // }

    return res;
}


/*****************************************************************************/
/* CAVariance */
float CAVariance::get(const cv::Mat& mat)
{
    using namespace cv;
    Mat mean_img, std_img;
    meanStdDev(mat, mean_img, std_img);
    return static_cast<float>(std_img.at<double>(0, 0));
}


/*****************************************************************************/
/* CAEngeryOfGradient */
float CAEngeryOfGradient::get(const cv::Mat& mat)
{
    using namespace cv;

    Mat k1 = (Mat_<char>(2, 1) << -1, 1);
    Mat k2 = (Mat_<char>(1, 2) << -1, 1);
    Mat e1, e2;
    filter2D(mat, e1, CV_32F, k1);
    filter2D(mat, e2, CV_32F, k2);
    Mat r = e1.mul(e1) + e2.mul(e2);
    return static_cast<float>(mean(r)[0]);
}


/*****************************************************************************/
/* CABrenner */
float CABrenner::get(const cv::Mat& mat)
{
    using namespace cv;
    Mat kb = (Mat_<char>(3, 1) << -1, 0, 1);
    Mat bi;
    filter2D(mat, bi, CV_32F, kb);
    pow(bi, 2, bi);
    return static_cast<float>(mean(bi)[0]);
}


/*****************************************************************************/
/* CALaplace */
float CALaplace::get(const cv::Mat& mat)
{
    using namespace cv;
    Mat kl = (Mat_<char>(3, 3) << -1, -1, -1, -1, 8, -1, -1, -1, -1);
    Mat li;
    filter2D(mat, li, CV_32F, kl);
    pow(li, 2, li);
    return static_cast<float>(mean(li)[0]);
}


/*****************************************************************************/
/* AutoFocusHillClimbing */
AutoFocusHillClimbing::AutoFocusHillClimbing(Box<ICVMatCreater> creater, Box<IContrastAlgo> algo,
                std::function<void(int)> move_cb,
                int min_pos, int max_pos, int fast_step,
                int normal_step, int skip_cnt)
:   _min_pos(min_pos), _max_pos(max_pos), _total_len(max_pos-min_pos+1),
    _skip_frame_cnt_max(skip_cnt)
{
    if (this->_total_len <= 0) {
        throw std::runtime_error("AutoFocus::_total_len(max_pos-min_pos+1) is invalid");
    }
    if (this->_skip_frame_cnt_max < 0) {
        throw std::runtime_error("AutoFocus::_skip_frame_cnt_max(skip_cnt) is invalid");
    }

    if (fast_step <= 0 || normal_step <= 0) {
        _step_info = AutoFocusStepInfo(this->_total_len);
        maix::log::info("AutoFocus use auto steps %d:%d-%d",
            this->_step_info.sf, this->_step_info.sn, this->_step_info.soft);
    } else if (fast_step > 0 && normal_step > 0) {
        _step_info.sf = fast_step;
        _step_info.sn = normal_step;
        _step_info.update(-1);
    } else {
        throw std::runtime_error("AutoFocus::_step_info(from fast_step and normal_step) is invalid");
    }

    this->move = move_cb;
    this->reset_creater(std::move(creater));
    this->reset_algo(std::move(algo));
    this->reset_status();
}

void AutoFocusHillClimbing::reset_creater(Box<ICVMatCreater> creater)
{
    this->_creater.release();
    this->_creater = std::move(creater);
}

void AutoFocusHillClimbing::reset_algo(Box<IContrastAlgo> algo)
{
    this->_algo.release();
    this->_algo = std::move(algo);
}

void AutoFocusHillClimbing::reset_status()
{
    this->_pos = this->_min_pos;
    this->_max_cla_pos = this->_pos;
    this->_max_cla_pos = 0;
    this->_skip_frame_cnt = 0;
    this->_max_cla = 0.0;
    this->_normal_max = 0;
    this->_mode = AFMode::RESET;
}

AutoFocusHillClimbing::AFMode AutoFocusHillClimbing::focus(ArcBox<image::Image> img)
{
    if (!img) {
        // maix::log::info("skip focus");
        return AFMode::SKIP;
    }

    if (!this->_creater || !this->_algo) {
        maix::log::warn("AutoFocus::focus cannot create cv::Mat, skip");
        return AFMode::SKIP;
    }

    dbg("focus mode = %d", static_cast<int>(this->_mode));

    AFMode res = AFMode::SKIP;
    switch (this->_mode) {
    case AFMode::FAST_FOCUS: {
        /* skip some frames */
        if (this->_skip_frame_cnt < this->_skip_frame_cnt_max) {
            this->_skip_frame_cnt++;
            res = AFMode::SKIP;
            break;
        }
        res = AFMode::FAST_FOCUS;
        this->_skip_frame_cnt = 0;

        /* create cv::Mat and get the contrast */
        auto data = this->_creater->get(img);
        auto cla = this->_algo->get(std::get<0>(data));

        dbg("FAST MODE cla=%0.2f", cla);

        if (cla > this->_max_cla) {
            this->_max_cla = cla;
            this->_max_cla_pos = this->_pos;
        }
        this->_pos += this->_step_info.sf;
        if (this->_pos >= this->_max_pos) {
            dbg("Fast focus finish, pos:  %d", this->_max_cla_pos);
            this->_pos = this->_max_cla_pos - this->_step_info.soft;
            this->_pos = std::max(1, this->_pos);
            this->_normal_max = this->_max_cla_pos + this->_step_info.soft;
            this->_normal_max = std::min(this->_max_pos, this->_normal_max);
            this->_skip_frame_cnt = 0;
            this->move(this->_pos-1);
            this->move(this->_pos-1);
            this->_mode = AFMode::NORMAL_FOCUS;
            break;
        }
        this->move(this->_pos-1);
        break;
    }
    case AFMode::NORMAL_FOCUS: {
        if (this->_skip_frame_cnt < this->_skip_frame_cnt_max) {
            this->_skip_frame_cnt++;
            res = AFMode::SKIP;
            break;
        }
        res = AFMode::NORMAL_FOCUS;
        this->_skip_frame_cnt = 0;
        auto data = this->_creater->get(img);
        auto cla = this->_algo->get(std::get<0>(data));

        dbg("NORMAL MODE cla=%0.2f", cla);

        if (cla > this->_max_cla) {
            this->_max_cla = cla;
            this->_max_cla_pos = this->_pos;
        }
        this->_pos += this->_step_info.sn;
        if (this->_pos >= this->_normal_max ||
            (this->_pos > (this->_normal_max - this->_step_info.soft)
             && cla <= this->_max_cla*0.75 )) {
            dbg("Nomal focus finish, pos: %d", this->_max_cla_pos);
            this->_skip_frame_cnt = 0;
            this->move(this->_max_cla_pos-1);
            this->move(this->_max_cla_pos-1);
            this->_mode = AFMode::STOP;
            break;
        }
        this->move(this->_pos-1);
        break;
    }
    case AFMode::STOP: {
        res = AFMode::STOP;
        this->reset_status();
        /* reset_status() will set mode to RESET */
        // this->_mode = AFMode::RESET;
        break;
    }
    case AFMode::RESET: {
        res = AFMode::RESET;
        this->move(this->_min_pos);
        this->_mode = AFMode::SKIP_FRAME_FIRST;
        break;
    }
    case AFMode::SKIP_FRAME_FIRST: {
        res = AFMode::SKIP_FRAME_FIRST;
        if (this->_skip_frame_cnt < this->_skip_frame_cnt_max) {
            this->_skip_frame_cnt++;
            break;
        }
        this->_skip_frame_cnt = 0;
        this->_mode = AFMode::FAST_FOCUS;
        break;
    }
    default: {
        res = AFMode::STOP;
        maix::log::warn("AutoFocus: unknown mode, try to reset mode...");
        this->reset_status();
        break;
    }
    }
    return res;
}


/*****************************************************************************/
/* Full Scan */
FullScan::FullScan(Box<ICVMatCreater> creater, Box<IContrastAlgo> algo,
    std::function<void(int)> move_cb, int min_pos, int max_pos, int step, int skip)
    : move(move_cb), _mode(Mode::STOP), _min(min_pos), _max(max_pos), _skip_cnt(0), _skip_cnt_max(skip),
      _creater(std::move(creater)), _algo(std::move(algo)) {}

void FullScan::reset_status() noexcept
{
    this->_mode = Mode::SKIP_FRAME_FIRST;
    this->_skip_cnt = 0;
    this->_pos = this->_min;
    this->_info.clear();
    this->_info.reserve(this->_max-this->_min+1);
}

void FullScan::set_save_path(const char* path)
{
    this->_save_path = std::string(path);
    if (!std::filesystem::exists(this->_save_path)) {
        std::filesystem::create_directory(this->_save_path);
    }
}

FullScan::Mode FullScan::scan(ArcBox<image::Image> img)
{
    if (!img) {
        return Mode::SKIP;
    }
    if (!this->_creater || !this->_algo) {
        maix::log::warn("AutoFocus::focus cannot create cv::Mat, skip");
        return Mode::SKIP;
    }

    dbg("mode = %d", static_cast<int>(this->_mode));

    Mode res = this->_mode;
    switch (this->_mode) {
    case Mode::STOP: {
        res = Mode::SKIP_FRAME_FIRST;
        this->reset_status();
        this->move(this->_min);
        this->_mode = Mode::SKIP_FRAME_FIRST;
        break;
    }
    case Mode::SKIP_FRAME_FIRST: {
        // res = this->_mode;
        if (this->_skip_cnt < this->_skip_cnt_max) {
            this->_skip_cnt++;
            break;
        }
        this->_skip_cnt = 0;
        this->_mode = Mode::SCAN;
        break;
    }
    case Mode::SCAN: {
        if (this->_skip_cnt < this->_skip_cnt_max) {
            this->_skip_cnt++;
            res = Mode::SKIP;
            break;
        }
        // res = Mode::SCAN;
        this->_skip_cnt = 0;
        auto data = this->_creater->get(img);
        auto cla = this->_algo->get(std::get<0>(data));

        dbg("%d cla=%0.2f", this->_pos, cla);

        this->_info.emplace_back(this->_pos, cla);
        if (!this->_save_path.empty()) {
            std::stringstream ss;
            ss << this->_save_path
                << "/" << "dis_"
                << std::setw(5) << std::setfill('0') << this->_pos
                << "_" << std::to_string(cla)
                << ".jpg";
            // std::get<1>(data)->save(ss.str().c_str());
            Box<image::Image> _img = Box<image::Image>(
                std::get<1>(data)->to_format(image::FMT_JPEG)
            );
            _img->save(ss.str().c_str());
        }
        this->_pos++;
        if (this->_pos >= this->_max) {
            this->_mode = Mode::STOP;
            res = Mode::STOP;
            break;
        }
        this->move(this->_pos-1);
        break;
    }
    default: {
        break;
    }
    };
    return res;
}

std::vector<FullScan::PointInfo> FullScan::result() noexcept
{
    return this->_info;
}

void FullScan::save(const char* path)
{
    std::ofstream ofs(path, std::ios_base::openmode::_S_trunc);
    if (!ofs.is_open()) {
        maix::log::error("[%s:%d] open file %s failed", __PRETTY_FUNCTION__, __LINE__, path);
        return;
    }

    for (auto [dis, cla] : this->_info) {
        std::stringstream ss;
        ss << std::to_string(dis) << ' ' << std::to_string(cla) << '\n';
        ofs.write(ss.str().c_str(), ss.str().size());
        if (!ofs.good()) {
            maix::log::warn("write [%d %f] failed", dis, cla);
        }
    }

    ofs.flush();
    ofs.close();
}


/*****************************************************************************/
/* AutoFocusHCFuture */
AutoFocusHCFuture::AutoFocusHCFuture(Box<ICVMatCreater> creater,
    Box<IContrastAlgo> algo, std::function<void(int)> move_cb,
    int min_pos, int max_pos, int fast_step, int normal_step, int skip)
    : move{move_cb}, _mode{Mode::FIRST_SKIP}, _min_pos{min_pos},
      _max_pos{max_pos}, _total_len{max_pos-min_pos+1},
      _normal_max{}, _pos{_min_pos}, _max_cla_pos{_pos},
      _skip_frame_cnt{}, _skip_frame_cnt_max{skip}, _max_cla{},
      _creater{std::move(creater)}, _algo{std::move(algo)}
{
    if (!this->_creater || !this->_algo) {
        throw std::runtime_error("AutoFocus::_creater|AutoFocus::_algo == nullptr!");
    }
    if (this->_total_len <= 0) {
        throw std::runtime_error("AutoFocus::_total_len <= 0!");
    }
    if (this->_skip_frame_cnt_max < 0) {
        throw std::runtime_error("AutoFocus::_skip_cnt_max < 0!");
    }

    if (fast_step <=0 || normal_step <= 0) {
        this->_step_info = AutoFocusStepInfo(this->_total_len);
        maix::log::info("AutoFocus use auto steps %d:%d-%d",
            this->_step_info.sf, this->_step_info.sn, this->_step_info.soft);
    } else if (fast_step > 0 && normal_step > 0) {
        _step_info.sf = fast_step;
        _step_info.sn = normal_step;
        _step_info.update(-1);
    }
}

void AutoFocusHCFuture::reset_status() noexcept
{
    this->_mode = Mode::FIRST_SKIP;
    this->_normal_max = {};
    this->_pos = this->_min_pos;
    this->_max_cla_pos = this->_pos;
    this->_skip_frame_cnt = {};
    this->_max_cla = {};
}

AutoFocusHCFuture::Mode AutoFocusHCFuture::focus(ArcBox<image::Image> img)
{
    Mode res = Mode::SKIP;
    if (!img) {
        return res;
    }

    dbg("mode = %d", static_cast<int>(this->_mode));
    res = this->_mode;

    switch (this->_mode) {
    case Mode::FIRST_SKIP: {
        if (this->_skip_frame_cnt < this->_skip_frame_cnt_max) {
            this->_skip_frame_cnt++;
            break;
        }
        this->_pos = this->_min_pos;
        this->move(this->_pos);
        this->_skip_frame_cnt = 0;
        this->_mode = Mode::FAST_FOCUS;
        break;
    }
    case Mode::FAST_FOCUS: {
        auto prev_pos = this->_pos;
        this->_pos += this->_step_info.sf;
        if (this->_pos <= this->_max_pos) {
            this->move(this->_pos-1);
            auto data = this->_creater->get(img);
            auto cla = this->_algo->get(std::get<0>(data));
            dbg("FAST MODE cla=%0.2f", cla);
            if (cla > this->_max_cla) {
                this->_max_cla = cla;
                this->_max_cla_pos = prev_pos;
            }
            break;
        }
        /* FIXME: skip last Sf */
        dbg("FAST finish, pos: %d", this->_max_cla_pos);
        this->_pos = this->_max_cla_pos - this->_step_info.soft;
        this->_pos = std::max(this->_min_pos+1, this->_pos);
        this->_normal_max = this->_max_cla_pos + this->_step_info.soft;
        this->_normal_max = std::min(this->_max_pos, this->_normal_max);

        this->_mode = Mode::NORMAL_FOCUS;

        break;
    }
    case Mode::NORMAL_FOCUS: {
        auto prev_pos = this->_pos;
        this->_pos += this->_step_info.sn;
        if (this->_pos <= this->_normal_max) {
            this->move(this->_pos-1);
            auto data = this->_creater->get(img);
            auto cla = this->_algo->get(std::get<0>(data));
            dbg("NORMAL MODE cla=%0.2f", cla);
            if (cla > this->_max_cla) {
                this->_max_cla = cla;
                this->_max_cla_pos = prev_pos;
            }
            break;
        }
        /* FIXME: skip last Sn */
        dbg("NORMAL finish, pos: %d", this->_max_cla_pos);
        this->move(this->_max_cla_pos-1);
        this->reset_status();
        res = Mode::STOP;
        this->_mode = Mode::FIRST_SKIP;
        break;
    }
    default:
        break;
    }
    return res;
}


AutoFocusHCFutureMini::AutoFocusHCFutureMini(Box<ICVMatCreater> creater,
    Box<IContrastAlgo> algo, std::function<void(int)> move_cb,
    int min_pos, int max_pos, int fast_step, int normal_step, int skip)
    : AutoFocusHCFuture(std::move(creater), std::move(algo), move_cb, min_pos,
      max_pos, fast_step, normal_step, skip) {}

AutoFocusHCFutureMini::Mode AutoFocusHCFutureMini::focus(ArcBox<image::Image> img)
{
    Mode res = Mode::SKIP;
    if (!img) {
        return res;
    }

    dbg("mode = %d", static_cast<int>(this->_mode));
    res = this->_mode;

    switch (this->_mode) {
    case Mode::FIRST_SKIP: {
        if (this->_skip_frame_cnt < this->_skip_frame_cnt_max) {
            this->_skip_frame_cnt++;
            break;
        }
        this->_pos = this->_min_pos;
        this->move(this->_pos);
        this->_skip_frame_cnt = 0;
        this->_mode = Mode::FAST_FOCUS;
        break;
    }
    case Mode::FAST_FOCUS: {
        auto prev_pos = this->_pos;
        this->_pos += this->_step_info.sf;
        if (this->_pos <= this->_max_pos) {
            this->move(this->_pos-1);

            // auto lt = maix::time::ticks_ms();
            auto data = this->_creater->get(img);
            // auto drt = maix::time::ticks_ms();
            auto cla = this->_algo->get(std::get<0>(data));
            // maix::log::info("create cv Mat used: %llu, calc used: %llu",
                // drt-lt, maix::time::ticks_ms()-drt);

            dbg("FAST MODE cla=%0.2f", cla);
            if (cla > this->_max_cla) {
                this->_max_cla = cla;
                this->_max_cla_pos = prev_pos;
            }
            break;
        }
        /* FIXME: skip last Sf */
        dbg("FAST finish, pos: %d", this->_max_cla_pos);
        this->_pos = this->_max_cla_pos - this->_step_info.soft;
        this->_pos = std::max(this->_min_pos+1, this->_pos);
        this->_normal_max = this->_max_cla_pos + this->_step_info.soft;
        this->_normal_max = std::min(this->_max_pos, this->_normal_max);

        this->_mode = Mode::NORMAL_FOCUS;

        break;
    }
    case Mode::NORMAL_FOCUS: {
        auto prev_pos = this->_pos;
        this->_pos += this->_step_info.sn;
        if (this->_pos <= this->_normal_max) {
            this->move(this->_pos-1);
            auto data = this->_creater->get(img);
            auto cla = this->_algo->get(std::get<0>(data));
            dbg("NORMAL MODE cla=%0.2f", cla);
            if (cla > this->_max_cla) {
                this->_max_cla = cla;
                this->_max_cla_pos = prev_pos;
            }
            if (!((this->_pos - this->_step_info.sn) > (this->_normal_max - this->_step_info.soft)
                && cla <= (this->_max_cla * 0.7))) {
                break;
            }
        }
        /* FIXME: skip last Sn */
        dbg("NORMAL finish, pos: %d", this->_max_cla_pos);
        this->move(this->_max_cla_pos-1);
        this->reset_status();
        res = Mode::STOP;
        this->_mode = Mode::FIRST_SKIP;
        break;
    }
    default:
        break;
    }
    return res;
}


}