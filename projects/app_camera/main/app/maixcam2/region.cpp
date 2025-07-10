#include "region.hpp"
#include "maix_basic.hpp"
#include "ax_middleware.hpp"

typedef struct {
    IVPS_RGN_HANDLE handle;
    AX_IVPS_RGN_DISP_GROUP_T rgn_grp;
    AX_U64 phy_addr;
    void *vir_addr;
} region_param_t;

static AX_BOOL LoadImage(const char *pszImge, AX_U32 pImgSize, AX_U64 *pPhyAddr, AX_VOID **ppVirAddr)
{
    AX_S32 ret;
    FILE *fp = fopen(pszImge, "rb");

    if (fp)
    {
        ret = fseek(fp, 0, SEEK_END);
        if (ret)
        {
            printf("fseek fail, ret=0x%x", ret);
        }
        AX_U32 nFileSize = ftell(fp);
        if (pImgSize > 0 && pImgSize != nFileSize)
        {
            printf("file size not right, %d != %d", pImgSize, nFileSize);
            fclose(fp);
            return AX_FALSE;
        }
        ret = fseek(fp, 0, SEEK_SET);
        if (ret)
        {
            printf("fseek fail, ret=0x%x", ret);
        }
        if (!nFileSize)
        {
            printf("%s nFileSize is 0 !", pszImge);
            return AX_FALSE;
        }
        ret = AX_SYS_MemAlloc((AX_U64 *)pPhyAddr, ppVirAddr, nFileSize, 16, NULL);
        if (0 != ret)
        {
            printf("AX_SYS_MemAlloc fail, ret=0x%x", ret);
            fclose(fp);
            return AX_FALSE;
        }
        if (fread(*ppVirAddr, 1, nFileSize, fp) != nFileSize)
        {
            printf("fread fail, %s", strerror(errno));
            fclose(fp);
            return AX_FALSE;
        }
        fclose(fp);

        return AX_TRUE;
    }
    else
    {
        printf("fopen %s fail, %s", pszImge, strerror(errno));
        return AX_FALSE;
    }
}

Region::Region(int x, int y, int width, int height, image::Format format, camera::Camera *camera)
{
    region_param_t *param = (region_param_t *)malloc(sizeof(region_param_t));
    err::check_null_raise(param, "malloc region param failed");
    memset(param, 0, sizeof(region_param_t));

    IVPS_RGN_HANDLE handle = AX_IVPS_RGN_Create();
    err::check_bool_raise(handle != AX_IVPS_INVALID_REGION_HANDLE, "AX_IVPS_RGN_Create failed");
    param->handle = handle;

    int flip = true;
    int mirror = true;
    auto configs = sys::device_configs(true);
    for (auto &item : configs) {
        log::info("device:%s value:%s", item.first.c_str(), item.second.c_str());
    }
    auto mirror_string = configs.find("cam_flip");
    auto flip_string = configs.find("cam_mirror");
    if (mirror_string != configs.end()) {
        mirror = !atoi(mirror_string->second.c_str());
    }

    if (flip_string != configs.end()) {
        flip = !atoi(flip_string->second.c_str());
    }

    int x2 = flip ? camera->width() - width - x : x;
    int y2 = mirror ? camera->height() - height - y : y;

    int ivps_grp = 0;
    int vi_vpss_chn = camera->get_channel();
    IVPS_FILTER ivps_filter = (vi_vpss_chn + 1) << 4;
    AX_S32 ax_ret = AX_IVPS_RGN_AttachToFilter(handle, ivps_grp, ivps_filter);
    err::check_bool_raise(ax_ret == AX_SUCCESS, "AX_IVPS_RGN_AttachToFilter failed");

    AX_U64 phy_addr = 0;
    void *vir_addr = NULL;
    ax_ret = AX_SYS_MemAlloc(&phy_addr, &vir_addr, width * height * 2, 512, NULL);
    if (ax_ret != AX_SUCCESS) {
        log::error("AX_SYS_MemAlloc failed, ret:%#x", ax_ret);
        err::check_bool_raise(ax_ret == AX_SUCCESS, "AX_SYS_MemAlloc failed");
    }

    param->vir_addr = vir_addr;
    param->phy_addr = phy_addr;

    this->_image = new image::Image(width, height, format);
    err::check_bool_raise(this->_image != nullptr, "region new image failed");

    AX_IVPS_RGN_DISP_GROUP_T *rgn_grp = &param->rgn_grp;
    AX_IVPS_RGN_DISP_T *disp = rgn_grp->arrDisp;
    rgn_grp->nNum = 1;
    rgn_grp->tChnAttr.nZindex = 0;
    rgn_grp->tChnAttr.nAlpha = 255;
    rgn_grp->tChnAttr.eFormat = AX_FORMAT_ARGB1555;
    disp[0].bShow = AX_TRUE;
    disp[0].eType = AX_IVPS_RGN_TYPE_OSD;
    disp[0].uDisp.tOSD.u16Alpha = 255;
    disp[0].uDisp.tOSD.enRgbFormat = AX_FORMAT_ARGB1555;
    disp[0].uDisp.tOSD.u32BmpWidth = width;
    disp[0].uDisp.tOSD.u32BmpHeight = height;
    disp[0].uDisp.tOSD.u32DstXoffset = x;
    disp[0].uDisp.tOSD.u32DstYoffset = y;
    disp[0].uDisp.tOSD.u64PhyAddr = param->phy_addr;
    disp[0].uDisp.tOSD.pBitmap = (AX_U8 *)param->vir_addr;

    this->_width = width;
    this->_height = height;
    this->_x = x;
    this->_y = y;
    this->_format = format;
    this->_camera = camera;
    this->_flip = flip;
    this->_mirror = mirror;

    _handle = param;
}
Region::~Region()
{
    region_param_t *param = (region_param_t *)_handle;
    IVPS_RGN_HANDLE handle = (IVPS_RGN_HANDLE)param->handle;
    AX_S32 ax_ret = AX_SUCCESS;
    int ivps_grp = 0;
    int vi_vpss_chn = _camera->get_channel();
    IVPS_FILTER ivps_filter = (vi_vpss_chn + 1) << 4;
    ax_ret = AX_IVPS_RGN_DetachFromFilter(handle, ivps_grp, ivps_filter);
    if (ax_ret != AX_SUCCESS) {
        log::error("AX_IVPS_RGN_DetachFromFilter failed!");
    }

    ax_ret = AX_IVPS_RGN_Destroy(handle);
    if (ax_ret != AX_SUCCESS) {
        log::error("AX_IVPS_RGN_Destroy failed!");
    }

    ax_ret = AX_SYS_MemFree(param->phy_addr, param->vir_addr);
    if (ax_ret != AX_SUCCESS) {
        log::error("AX_SYS_MemFree failed! ret:%#x", ax_ret);
    }

    if (_handle) {
        free(_handle);
        _handle = nullptr;
    }
}

image::Image *Region::get_canvas()
{
    return this->_image;
}

// 单个像素转换：BGRA8888 → ARGB1555
uint16_t bgra8888_to_argb1555(uint8_t b, uint8_t g, uint8_t r, uint8_t a) {
    uint16_t alpha = (a > 127) ? 1 : 0;
    uint16_t r5 = r >> 3;
    uint16_t g5 = g >> 3;
    uint16_t b5 = b >> 3;

    return (alpha << 15) | (r5 << 10) | (g5 << 5) | b5;
}

// 整体图像转换（BGRA数据 → ARGB1555数据）
void convert_image_bgra8888_to_argb1555(uint8_t* src_bgra, uint16_t* dst_argb1555, int pixel_count) {
    for (int i = 0; i < pixel_count; ++i) {
        uint8_t b = src_bgra[i * 4 + 0];
        uint8_t g = src_bgra[i * 4 + 1];
        uint8_t r = src_bgra[i * 4 + 2];
        uint8_t a = src_bgra[i * 4 + 3];
        dst_argb1555[i] = bgra8888_to_argb1555(b, g, r, a);
    }
}

err::Err Region::update_canvas()
{
    AX_S32 ax_ret = AX_SUCCESS;
    region_param_t *param = (region_param_t *)_handle;
    image::Image *img = this->_image;

    if (img->format() == image::Format::FMT_BGRA8888) {
        // bgra8888 to argb1555
        uint8_t *src_u8 = (uint8_t *)img->data();
        uint16_t *dst_u16 = (uint16_t *)param->vir_addr;
        convert_image_bgra8888_to_argb1555(src_u8, dst_u16, _width * _height);
    }
    ax_ret = AX_IVPS_RGN_Update(param->handle, &param->rgn_grp);
    if (ax_ret != AX_SUCCESS) {
        log::error("AX_IVPS_RGN_Update failed! ret:%#x\r\n", ax_ret);
        return err::ERR_RUNTIME;
    }

    return err::ERR_NONE;
}