#ifndef __MAIX_FOCUS_HPP__
#define __MAIX_FOCUS_HPP__

#include <opencv2/opencv.hpp>
#include <memory>
#include <stdexcept>

#include "maix_image.hpp"

#define MAIX_FOCUS_NAMESPACE maix::focus

namespace MAIX_FOCUS_NAMESPACE {

/* std::unique_ptr is to long */
template<typename T>
using Box = std::unique_ptr<T>;

/* std::shared_ptr is to long */
template<typename T>
using ArcBox = std::shared_ptr<T>;


/********************************************************/
        /* Class for focusing assistance. */
/**
 * @brief AutoFocusStepInfo
 *  The minimum total number of steps to
 *  obtain focus from the total length.
 */
struct AutoFocusStepInfo {
    int lf;
    int sf;
    int sn;
    int soft;
    AutoFocusStepInfo() = default;
    explicit AutoFocusStepInfo(int lf) noexcept;
    AutoFocusStepInfo(int sf, int sn) noexcept;
    void update(int lf) noexcept;
};

/**
 * @brief PointMapper
 *
 */
class PointMapper {
public:
    PointMapper(int src_w, int src_h, int dst_w, int dst_h) noexcept;
    std::pair<int, int> map_s2d(int sx, int sy) noexcept;
    std::pair<int, int> map_d2s(int dx, int dy) noexcept;
private:
    int _sw;
    int _sh;
    int _dw;
    int _dh;
};

/********************************************************/
        /* Interface */
/**
 * @brief IContrastAlgo
 *  Contrast Algo Interface
 */
class IContrastAlgo {
public:
    virtual float get(const cv::Mat& mat) = 0;
    virtual ~IContrastAlgo() {}
};

/**
 * @brief ICVMatCreater
 *  cv::Mat Creater Interface
 */
class ICVMatCreater {
public:
    using Image = maix::image::Image;
    using RT = std::pair<cv::Mat, ArcBox<Image>>;
    virtual RT get(ArcBox<Image> img) = 0;
    virtual ~ICVMatCreater() {}
};

/**
 * @brief IImageCreater
 *  Image Creater Interface
 */
class IImageCreater {
public:
    using Image = maix::image::Image;
    virtual ArcBox<Image> create(ArcBox<Image> img) = 0;
    virtual ~IImageCreater() {}
};

/**
 * @brief IImageCropper
 *  Image Cropper Interface
 */
class IImageCropper : public IImageCreater {
public:
    struct Rect {
        int x;
        int y;
        int w;
        int h;
        Rect() noexcept;
        Rect(int x, int y, int w, int h) noexcept;
    };

public:
    virtual ArcBox<Image> create(ArcBox<Image> img) = 0;
    virtual Rect rect(int cx, int cy, int w, int h, int iw, int ih);
    virtual Rect rect(const Rect& cr, int iw, int ih);
    virtual ~IImageCropper() {}
};


/********************************************************/
        /* CVMatCreater Impl */
/**
 * @brief CVMatCreater
 *  create cv::Mat from maix::image::Image
 */
class CVMatCreater final : public ICVMatCreater {
public:
    CVMatCreater() noexcept;
    CVMatCreater(Box<IImageCreater> img_creater);
    virtual RT get(ArcBox<Image> img) override;
private:
    Box<IImageCreater> _creater;
};

/********************************************************/
        /* ImageCreater Impl */
/**
 * @brief ImageForward
 *  forward maix::image::Image
 */
class ImageForward final : public IImageCreater {
public:
    virtual ArcBox<Image> create(ArcBox<Image> img) override;
};

/**
 * @brief ImageGrayCreater
 *  create FMT_GRAY image from image
 */
class ImageGrayCreater final : public IImageCreater {
public:
    ImageGrayCreater() noexcept;
    ImageGrayCreater(Box<IImageCreater> prev_creater);
    virtual ArcBox<Image> create(ArcBox<Image> img) override;
private:
    Box<IImageCreater> _prev;
};


/********************************************************/
        /* IImageCropper Impl */
/**
 * @brief ImageCropper
 *  normal cropper
 */
class ImageCropper : public IImageCropper {
public:
    ImageCropper(int cx, int cy, int w, int h) noexcept;
    ImageCropper(int cx, int cy, int w, int h, Box<IImageCreater> prev_creater);
    virtual ArcBox<Image> create(ArcBox<Image> img) override;
    virtual Rect rect(int cx, int cy, int w, int h, int iw, int ih) override;
    virtual Rect rect(const Rect& cr, int iw, int ih) override;
private:
    Rect _cr;
    Box<IImageCreater> _prev;
};

/**
 * @brief ImageYVU420SPNV21Cropper
 *  YUV420SP NV21 cropper
 */
class ImageYVU420SPNV21Cropper : public IImageCropper {
public:
    ImageYVU420SPNV21Cropper(int cx, int cy, int w, int h) noexcept;
    ImageYVU420SPNV21Cropper(int cx, int cy, int w, int h, Box<IImageCreater> prev_creater);
    virtual ArcBox<Image> create(ArcBox<Image> img) override;
    virtual Rect rect(int cx, int cy, int w, int h, int iw, int ih) override;
    virtual Rect rect(const Rect& cr, int iw, int ih) override;
private:
    Rect _cr;
    Box<IImageCreater> _prev;
};

/********************************************************/
        /* IContrastAlgo Impl */
/**
 * @brief CAVariance
 *  Variance
 */
class CAVariance final : public IContrastAlgo {
public:
    virtual float get(const cv::Mat& mat) override;
};

/**
 * @brief CAEngeryOfGradient
 *  EngeryOfGradient
 */
class CAEngeryOfGradient final : public IContrastAlgo {
public:
    virtual float get(const cv::Mat& mat) override;
};

/**
 * @brief CABrenner
 *  Brenner
 */
class CABrenner final : public IContrastAlgo {
public:
    virtual float get(const cv::Mat& mat) override;
};

/**
 * @brief CALaplace
 *  Laplace
 */
class CALaplace final : public IContrastAlgo {
public:
    virtual float get(const cv::Mat& mat) override;
};


/********************************************************/
        /* Scan Impl */
/**
 * @brief FullScan
 *
 */
class FullScan {
public:
    enum class Mode {
        STOP,
        SKIP_FRAME_FIRST,
        SCAN,
        SKIP
    };
    struct PointInfo {
        int pos;
        float cla;
        PointInfo(int pos, float cla)
            : pos(pos), cla(cla) {}
    };
public:
    FullScan(Box<ICVMatCreater> creater,
            Box<IContrastAlgo> algo,
            std::function<void(int)> move_cb,
            int min_pos, int max_pos, int step, int skip);
    void set_save_path(const char* path);
    Mode scan(ArcBox<image::Image> img);
    std::vector<PointInfo> result() noexcept;
    void save(const char* path);

private:
    void reset_status() noexcept;
    std::function<void(int)> move;
private:
    Mode _mode;
    int _min;
    int _max;
    int _skip_cnt;
    int _skip_cnt_max;
    int _pos;
    std::string _save_path;
    Box<ICVMatCreater> _creater;
    Box<IContrastAlgo> _algo;
    std::vector<PointInfo> _info;
};

/********************************************************/
        /* AutoFocus Impl */
/**
 * @brief AutoFocusHillClimbing
 *  HillClimbing
 */
class AutoFocusHillClimbing {
public:
    enum class AFMode {
        STOP,
        RESET,
        SKIP_FRAME_FIRST,
        FAST_FOCUS,
        NORMAL_FOCUS,
        SKIP,
    };
public:
    AutoFocusHillClimbing(Box<ICVMatCreater> creater, Box<IContrastAlgo> algo,
                std::function<void(int)> move_cb,
                int min_pos, int max_pos, int fast_step=-1,
                int normal_step=-1, int skip_cnt=2);
    void reset_creater(Box<ICVMatCreater> creater);
    void reset_algo(Box<IContrastAlgo> algo);
    void reset_status();

    AFMode focus(ArcBox<image::Image> img);
private:
    /* ABS */
    std::function<void(int)> move;
private:
    AFMode _mode;
    int _min_pos;
    int _max_pos;
    int _total_len;
    int _normal_max;
    int _pos;
    int _max_cla_pos;
    int _skip_frame_cnt;
    int _skip_frame_cnt_max;
    float _max_cla;
    AutoFocusStepInfo _step_info;
    Box<ICVMatCreater> _creater;
    Box<IContrastAlgo> _algo;
};

/**
 * @brief AutoFocusHCFuture
 *  HillClimbing + Future
 */
class AutoFocusHCFuture {
public:
    enum class Mode {
        STOP = 0,
        FIRST_SKIP,
        FAST_FOCUS,
        NORMAL_FOCUS,
        SKIP,
    };
public:
    AutoFocusHCFuture(Box<ICVMatCreater> creater,
                Box<IContrastAlgo> algo,
                std::function<void(int)> move_cb,
                int min_pos, int max_pos, int fast_step=-1,
                int normal_step=-1, int skip=2);
    Mode focus(ArcBox<image::Image> img);
    void reset_status() noexcept;
protected:
    std::function<void(int)> move;
protected:
    Mode _mode;
    int _min_pos;
    int _max_pos;
    int _total_len;
    int _normal_max;
    int _pos;
    int _max_cla_pos;
    int _skip_frame_cnt;
    int _skip_frame_cnt_max;
    float _max_cla;
    AutoFocusStepInfo _step_info;
    Box<ICVMatCreater> _creater;
    Box<IContrastAlgo> _algo;
};

/**
 * @brief AutoFocusHCFutureMini
 *  HillClimbing + Future, and skip some normal step
 */
class AutoFocusHCFutureMini : public AutoFocusHCFuture {
public:
    AutoFocusHCFutureMini(Box<ICVMatCreater> creater,
                Box<IContrastAlgo> algo,
                std::function<void(int)> move_cb,
                int min_pos, int max_pos, int fast_step=-1,
                int normal_step=-1, int skip=2);
    Mode focus(ArcBox<image::Image> img);
};


}



#endif // __MAIX_FOCUS_HPP__