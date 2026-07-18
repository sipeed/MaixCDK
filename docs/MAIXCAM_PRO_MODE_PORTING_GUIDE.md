# MaixCAM Pro (SG2002 + OS04A10) 模式适配指南

> 本文档总结为 MaixCAM Pro (SG2002/CV1813H + OS04A10 传感器) 适配新分辨率/帧率模式的经验教训。

---

## 一、架构总览

### 视频管线

```
Sensor → MIPI RX (CSI) → VI (Pipe + ISP) → VPSS → 应用层 (cam.read())
 ① I2C    ② devmem       ③ SAMPLE_PLAT_VI_INIT   ④ CVI_VPSS_*
     配置      配置               + CVI_VI_*              + VB Pool
```

**四个需要改动的主要模块**：

| 模块 | 位置 | 涉及文件 |
|------|------|---------|
| ① 传感器驱动 | `libsophgo-middleware.so` | `os04a10_cmos_ex.h`, `os04a10_cmos_param.h`, `os04a10_sensor_ctl.c`, `os04a10_cmos.c` |
| ② MIPI RX 配置 | 传感器驱动 `sensor_rx_attr()` | `os04a10_cmos.c:1315` |
| ③ VI/ISP 初始化 | `libmaix.so` + `libsophgo-middleware.so` | `maix_camera_mmf.cpp:` `_mmf_vi_init()` |
| ④ VPSS + VB Pool | `libmaix.so` | `maix_camera_mmf.cpp:` `Camera::open()` |

### 已实现的模式

| 模式 | 分辨率 | 帧率 | HTS | VTS | SCLK | 状态 |
|------|--------|------|-----|-----|------|------|
| 1440p30 线性 | 2560×1440 | 30 | 1484 | 2432 | 104 MHz | ✅ 生产就绪 |
| 1080p60 线性 | 1920×1080 | 60 | 1484 | 1216 | 104 MHz | ✅ 生产就绪 |
| 720p90 线性 | 1280×720 | 90 | 1400 | 780 | 104 MHz | ✅ 已验证 |
| 1440p30 WDR | 2560×1440 | 30 | 2972 | 1624 | 104 MHz | ✅ 已验证 |

---

## 二、新增模式的改动清单

### 2.1 传感器驱动层（`libsophgo-middleware.so`）

需要修改四个文件（约 40 行）：

#### 文件 1: `os04a10_cmos_ex.h` — 添加枚举

```c
typedef enum _OS04A10_MODE_E {
    OS04A10_MODE_1440P30_12BIT = 0,
    OS04A10_MODE_NEWMODE,          // ← 新增
    OS04A10_MODE_1080P60_12BIT,
    OS04A10_MODE_LINEAR_NUM,
    OS04A10_MODE_1440P30_WDR = OS04A10_MODE_LINEAR_NUM,
    OS04A10_MODE_NUM
} OS04A10_MODE_E;
```

- 线性模式加在 `OS04A10_MODE_LINEAR_NUM` 之前
- WDR 模式加在 `OS04A10_MODE_LINEAR_NUM` 之后

#### 文件 2: `os04a10_cmos_param.h` — 参数表

```c
[OS04A10_MODE_NEWMODE] = {
    .name = "720p90_12bit",
    .astImg[0] = {
        .stSnsSize  = { .u32Width = 1280, .u32Height = 720 },
        .stWndRect  = { .s32X = 0, .s32Y = 0, .u32Width = 1280, .u32Height = 720 },
        .stMaxSize  = { .u32Width = 1280, .u32Height = 720 },
    },
    .f32MaxFps  = 90,          // ← 务必设置实际帧率！AE 依赖此值
    .f32MinFps  = 2.22,        // VTS * f32MaxFps / 0xFFFF
    .u32HtsDef  = 1400,
    .u32VtsDef  = 780,
    .stExp[0]   = { .u16Min = 1, .u16Max = 780 - 8, .u16Def = 500, .u16Step = 1 },
    // ... 其他参数从相近模式复制
};
```

**⚠️ `f32MaxFps` 必须设为实际帧率，不能偷懒写 30。** AE 和 `cmos_fps_set` 用此值计算 VTS。设 30 则最高只能跑 30fps。

#### 文件 3: `os04a10_sensor_ctl.c` — 初始化函数

```c
static void os04a10_linear_NEWMODE_init(VI_PIPE ViPipe)
{
    // 可参考同分辨率/相近帧率的 init 函数修改
    // 关键寄存器组：
    // 0x0300-0x030A: PLL1 (PCLK/MIPI)
    // 0x0322-0x032E: PLL2 (SCLK) ← 所有线性模式共用，通常不改
    // 0x3800-0x380B: 裁剪+输出尺寸
    // 0x380C-0x380D: HTS
    // 0x380E-0x380F: VTS
    // 0x3814-0x3817: 子采样 (binning/skipping)
    // 0x4800-0x4837: MIPI 配置

    // ...
    os04a10_default_reg_init(ViPipe);
    os04a10_write_register(ViPipe, 0x0100, 0x01);  // start streaming
}
```

然后在 `os04a10_init()` 分发函数中添加分支：

```c
void os04a10_init(VI_PIPE ViPipe) {
    // ...
    if (enWDRMode == WDR_MODE_2To1_LINE) {
        if (u8ImgMode == OS04A10_MODE_1440P30_WDR) ...
    } else {
        if (u8ImgMode == OS04A10_MODE_NEWMODE)
            os04a10_linear_NEWMODE_init(ViPipe);
        // ...
    }
}
```

**⚠️ 确保 init 函数中设置了正确的 HTS/VTS！** 如果从已有的 init 函数复制修改，确认 HTS/VTS 被覆写为新值（而非继承旧模式的时序）。

#### 文件 4: `os04a10_cmos.c` — 运行时支持

**`cmos_set_image_mode()`**（约 1190 行）— 添加分辨率/帧率匹配分支：

```c
} else if (pstSensorImageMode->f32Fps <= 90) {  // ← 新分支
    if (OS04A10_RES_IS_720P(...))
        u8SensorImageMode = OS04A10_MODE_720P90_12BIT;
}
```

**⚠️ 必须为高帧率添加新的 fps 分支！** 原始代码只有 `fps <= 30` 和 `fps <= 60` 两个分支，`fps > 60` 直接 `return CVI_FAILURE`。

**`sensor_global_init()`** — 调试用途可添加 `/tmp/force_xxx` 覆写：

```c
pstSnsState->u8ImgMode = OS04A10_MODE_1440P30_12BIT;
if (access("/tmp/force_720p90", F_OK) == 0)
    pstSnsState->u8ImgMode = OS04A10_MODE_720P90_12BIT;
```

**`sensor_rx_attr()`** — 如果新模式需要不同的 MIPI 参数（hs_settle/mac_clk/data_type），在此添加。

### 2.2 应用层（`libmaix.so`）

#### `maix_camera_mmf.cpp` — FPS 钳位

构造函数中（约 110 行）：

```cpp
// 默认 fps
if (fps == -1 && _width <= 1280 && _height <= 720)
    _fps = 60;
else if (fps == -1 && _width <= 1920 && _height <= 1080)
    _fps = 60;
else if (fps == -1)
    _fps = 30;

// fps 钳位
if (...) ...
else if (_width <= 1280 && _height <= 720 && _fps > 90)
    _fps = 90;
```

**⚠️ 原始代码的 720p 钳位逻辑有 bug：只允许 80fps，且 >60 但不等于 80 时钳到 80。** 需要改为：

```cpp
_fps > 90 → 90
_fps 30-90 之间的非标准值 → 90
```

#### `maix_camera_mmf.cpp` — 传感器类型选择

`_get_sensor_name()` 内部（约 580 行和 670 行）：

```cpp
if (width <= 1280 && height <= 720 && fps >= 80) {
    sensor_cfg.sns_type = OV_OS04A10_MIPI_4M_720P90_12BIT;
    CVI_BIN_SetBinName(WDR_MODE_NONE, "/mnt/cfg/param/cvi_sdr_bin.os04a10");
}
```

**⚠️ ISP bin 路径：** `maixcam_pro` 分支曾引用不存在的 `cvi_sdr_bin_90fps.os04a10`，必须改为标准 bin。ISP bin 的 `f32FrameRate` 会被 `CVI_ISP_SetPubAttr` 覆盖。

#### `maix_camera_mmf.cpp` — `_mmf_vi_init` 中的 AE 防护

在 `SAMPLE_PLAT_VI_INIT` 之后（约 826 行）：

```cpp
// 强制 ISP 帧率为实际帧率（bin 加载后可能被覆写）
ISP_PUB_ATTR_S stPubAttr;
CVI_ISP_GetPubAttr(0, &stPubAttr);
stPubAttr.f32FrameRate = fps;
CVI_ISP_SetPubAttr(0, &stPubAttr);

// 限制 AE 最大曝光时间，防止 VTS 增长
ISP_EXPOSURE_ATTR_S exp;
CVI_ISP_GetExposureAttr(0, &exp);
exp.stAuto.stExpTimeRange.u32Max = (CVI_U32)(1000000.0 / fps * 0.9);
CVI_ISP_SetExposureAttr(0, &exp);
```

---

## 三、关键陷阱与绕道方案

### 陷阱 1: 闭源库 `libmaixcam_lib.so`

**问题**：`mmf_init_v2()` + `mmf_vi_init_v2()` + `mmf_add_vi_channel_v2()` 是闭源库函数，存在：
- VB 池双初始化耗尽 → SIGSEGV
- WDR 双 pipe 需要闭库特殊逻辑
- 无法修改源码

**绕道**：用开源 bypass 完全替换：

```
替换 mmf_init_v2(false)    → 删除（使用启动 VB 池即可）
替换 mmf_vi_init_v2()     → SAMPLE_PLAT_VI_INIT()
替换 mmf_add_vi_channel_v2() → SAMPLE_PLAT_VPSS_INIT() + CVI_VPSS_CreatePool() + AttachVbPool()
```

**注意**：开源 bypass 后，`mmf_deinit_v2(false)` 在 `close()` 中仍在使用（因为它处理了传感器/MIPI 的停止顺序），但第二次 `open()` 会因 VB 池不释放块而失败。模式切换需要重启进程。

### 陷阱 2: VB 池（Video Buffer Pool）

**问题**：
- 启动时内核创建 3 块 × 5.6 MB 的 VB 池
- `SAMPLE_COMM_SYS_Init()` 试图销毁并重建池，但内核模块引用阻止销毁
- `CVI_VB_Exit()` 返回成功但实际不释放

**绕道**：**不要调用 `SAMPLE_COMM_SYS_Init()`**。直接使用启动池：

```cpp
// ❌ 不可行：SAMPLE_COMM_SYS_Init(&vb) 无法重设运行时池
// ✅ 跳过：使用启动池（3块 × 5.6MB）
```

启动池 3 块对于常见模式均充足：

| 模式 | 每帧大小 | 最多缓冲帧数 |
|------|---------|-------------|
| 720p90 | 1.4 MB | 12 帧 |
| 1080p60 | 3.1 MB | 5 帧 |
| 1440p30 | 5.5 MB | 3 帧 |

### 陷阱 3: `SAMPLE_COMM_VI_IniToViCfg()` 不处理 WDR 双 Pipe

**问题**：无论 WDR 还是线性，`IniToViCfg` 始终设置 `aPipe[1..5] = -1`。WDR 需要 2 个 pipe。

**绕道**：在 `IniToViCfg` 之后、`SAMPLE_PLAT_VI_INIT` 之前手动设置：

```cpp
if (wdr_mode) {
    stViConfig.astViInfo[0].stPipeInfo.aPipe[1] = 1;
    CVI_VI_SetDevAttr(0, &stWdrDevAttr);  // 设 enWDRMode
}
```

### 陷阱 4: `cmos_set_image_mode()` 没有高 fps 分支

**问题**：原始代码只有 `fps <= 30` 和 `fps <= 60` 两个分支，60+ 直接返回失败。

**绕道**：添加 `fps <= 90` 分支：

```c
} else if (pstSensorImageMode->f32Fps <= 90) {
    if (OS04A10_RES_IS_720P(...))
        u8SensorImageMode = OS04A10_MODE_720P90_12BIT;
}
```

### 陷阱 5: ISP Bin 加载后覆盖 `f32FrameRate`

**问题**：`SAMPLE_PLAT_VI_INIT` 内部调用 `CVI_ISP_LoadBin()`，bin 中的 `ISP_PUB_ATTR_S.f32FrameRate=30` 覆盖代码设置的 90。

**绕道**：在 `SAMPLE_PLAT_VI_INIT` 之后再次设置：

```cpp
CVI_ISP_GetPubAttr(0, &stPubAttr);
stPubAttr.f32FrameRate = fps;
CVI_ISP_SetPubAttr(0, &stPubAttr);
```

### 陷阱 6: AE 自动降低帧率

**问题**：AE（自动曝光）在暗光下降低目标帧率以增加曝光时间，`cmos_fps_set` 计算 `VMAX = VTS * maxFps / f32Fps`，降低后的 fps 导致 VTS 增大，帧率下降。

**绕道**：两层防护：

1. **限制最大曝光时间** —— `exp.stAuto.stExpTimeRange.u32Max = 1e6/fps * 0.9`
2. **强制 ISP 帧率** —— `CVI_ISP_SetPubAttr(f32FrameRate=fps)`

**I2C 写入不工作**：AE 通过 `cmos_fps_set` 持续更新 VTS，I2C 写入会被下一次 AE 更新覆盖。正确方法是在 ISP 层控制（`SetPubAttr` + `SetExposureAttr`）。

### 陷阱 7: `mmf_deinit_v2` 不完整清理

**问题**：`mmf_deinit_v2(false)` 不销毁 VPSS 组，不释放 VB 块。第二次 `open()` 时 VPSS 组已存在导致 `CreateGrp` 失败或 SIGSEGV。

**限制**：这是内核驱动 `soph_base.ko` 的底层限制，VB 块在运行时不可回收。**模式切换必须在独立进程中**。

### 陷阱 8: `string::c_str()` Use-After-Free

```cpp
// ❌ 临时 std::string 在分号后析构
const char *board_id = sys::device_id().c_str();

// ✅ 先存到 std::string
std::string s = sys::device_id();
const char *board_id = s.c_str();
```

### 陷阱 9: pybind11 Wrapper 覆盖默认格式

`maixpy_wrapper.cpp`（CMake 生成）中 Camera 构造函数的 Python 默认格式硬编码为 `image::FMT_RGB888`，覆盖 C++ 头文件的 `FMT_YVU420SP`。每次 cmake reconfigure 后需手动修复。

### 陷阱 10: `snsr_type_name[]` 数组断裂

`sample_common_sensor.c` 中的 `snsr_type_name[]` 字符串数组必须与 `SAMPLE_SNS_TYPE_E` 枚举一一对应。缺少逗号或缺少条目会导致 `decode: unknown` 错误。

### 陷阱 11: PLL 分频器编码

`0x032A` (pll2_divt) 的编码：

| 寄存器值 | 分频比 |
|---------|--------|
| 0 | /1 |
| 1 | /1.5 |
| **2** | **/2.5** ← 不是 /2！ |
| 3 | /2 |
| 4 | /3 |

**编码 2 = /2.5，不是 /2。** 计算 SCLK 时务使用正确编码。

SCLK 计算公式：
```
PLL2_VCO = EXTCLK / predivp / prediv × mult
SCLK = PLL2_VCO / divst / divt
```

其中 `divt` 是 `0x032A` 的分频比（2 → /2.5），`divst` 是 `0x0328` 的值（1~16）。

### 陷阱 12: 传感器寄存器写入时序

- VTS 等时序寄存器应通过 `cmos_fps_set()` 的 blanking 更新路径写入，而非在 `os04a10_default_reg_init()` 中直接写入
- 直接 SCCB 写入"不保证在帧边界生效"（数据手册）
- WDR 模式下 VTS 写入时序有未文档化的依赖，写入错误值可能导致管���不工作

---

## 四、推荐开发流程

### 步骤 1: 计算时钟和时序

```
目标: 720p90
SCLK = 104 MHz (现有 PLL 配置)
HTS × VTS = SCLK / fps = 104M / 90 = 1,155,556
选择 HTS = 1400 → VTS = 1155556 / 1400 ≈ 825
检查 VTS >= 输出高度 + 消隐 = 720 + 60 = 780
```

### 步骤 2: 修改传感器驱动

```
os04a10_cmos_ex.h      → 枚举
os04a10_cmos_param.h    → 参数表（注意 f32MaxFps！）
os04a10_sensor_ctl.c    → init 函数（注意 HTS/VTS 覆写！）
os04a10_cmos.c          → cmos_set_image_mode（注意 fps 分支！）
```

### 步骤 3: 修改应用层

```
maix_camera_mmf.cpp     → FPS 钳位 + 传感器选择 + AE 防护 + ISP bin 路径
```

### 步骤 4: 编译部署

```bash
cd ~/sg2002/MaixPy/build
cmake .. -DCONFIG_MAIXCAM_PRO=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make sophgo-middleware -j$(nproc)
make maix -j$(nproc)
scp sophgo-middleware/libsophgo-middleware.so root@IP:/usr/lib/python3.11/site-packages/maix/dl_lib/
scp maix/libmaix.so root@IP:/usr/lib/python3.11/site-packages/maix/_maix.so
```

### 步骤 5: 验证

```bash
# 杀死占用摄像头资源的后台服务
ssh root@IP "killall maixvision_server launcher_daemon"

# 测试
python3 -c "
from maix import camera
cam = camera.Camera(1280, 720, fps=90)
cam.set_fps(90)
for i in range(10):
    img = cam.read()
    if img: print(f'frame {i}: {img.width()}x{img.height()}')
"

# 寄存器回读验证
i2ctransfer -y -f 4 w2@0x36 0x38 0x0c r2   # HTS
i2ctransfer -y -f 4 w2@0x36 0x38 0x0e r2   # VTS
devmem 0x0A0D0390 32                         # PHY CK state
```

### 步骤 6: 每次 I2C 写入后回读确认

寄存器写入后立即通过 `i2ctransfer` 回读，确保值正确写入且未被意外覆盖。

---

## 五、关键文件路径速查

| 文件 | 路径 |
|------|------|
| 传感器枚举 | `MaixCDK/components/3rd_party/.../sg200x/ov_os04a10/os04a10_cmos_ex.h` |
| 传感器参数表 | `MaixCDK/.../sg200x/ov_os04a10/os04a10_cmos_param.h` |
| 传感器初始化 | `MaixCDK/.../sg200x/ov_os04a10/os04a10_sensor_ctl.c` |
| 传感器运行时 | `MaixCDK/.../sg200x/ov_os04a10/os04a10_cmos.c` |
| 摄像头管线 | `MaixCDK/components/vision/port/maixcam/maix_camera_mmf.cpp` |
| 摄像头头文件 | `MaixCDK/components/vision/include/maix_camera.hpp` |
| 传感器枚举映射 | `MaixCDK/.../sample/common/sample_common_sensor.c` |
| 传感器枚举定义 | `MaixCDK/.../sample/common/sample_comm.h` |
| ISP Bin 文件（板端） | `/mnt/cfg/param/cvi_sdr_bin.os04a10` |
| WDR ISP Bin（板端） | `/mnt/cfg/param/cvi_wdr_bin.os04a10` |

---

## 六、闭源库 vs 开源 bypass 对照表

| 功能 | 闭源库 | 开源 bypass |
|------|--------|-------------|
| VB 池 | `mmf_init_v2(false)` | 使用启动池（3 块 × 5.6 MB） |
| VI 初始化 | `mmf_vi_init_v2()` | `SAMPLE_PLAT_VI_INIT()` |
| VI 通道添加 | `mmf_add_vi_channel_v2()` | `CVI_VPSS_CreateGrp()` + `CVI_VB_CreatePool()` + `AttachVbPool()` + `SAMPLE_COMM_VI_Bind_VPSS()` |
| 清理 | `mmf_deinit_v2(false)` | **必须用闭源**（内核停止顺序复杂） |
| WDR 双 pipe | 内部自动处理 | 手动 `aPipe[1]=1` |
| MIPI RX 重配 | 内部处理 | 外部调用 `CVI_MIPI_SetMipiAttr()` |
| 依赖 | `libmaixcam_lib.so` (闭源) | `libsophgo-middleware.so` (开源) |
