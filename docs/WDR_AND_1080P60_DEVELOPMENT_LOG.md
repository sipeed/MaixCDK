# OS04A10 1080p60 + WDR 模式开发记录

> 最后更新: 2026-07-11 | 设备: MaixCAM Pro (SG2002/CV1813H) + OS04A10
> 
> 当前状态: **WDR 1440p30 完整 VI→VPSS→cam.read() 管道已通过验证**

---

## 一、1080p60 模式（已部署，生产就绪）

### 目标
在 MaixCAM Pro 上为 OS04A10 传感器添加真正的 1920×1080 @ 60fps 模式，完整 VI→ISP→VPSS 管道。

### 已修复的 Bug

| # | 问题 | 根因 | 修复 |
|---|------|------|------|
| 1 | `decode: unknown` + SIGSEGV | `snsr_type_name[]` 数组断裂（缺逗号+缺 OV2685 条目+无 NULL 防护） | `sample_common_sensor.c` 补逗号+加条目+加 NULL 检查 |
| 2 | `CVI_ERR_VB_NOBUF` | `mmf_init_v2(false)` + `mmf_vi_init_v2()` 双初始化耗尽 VB 池 | 在 `mmf_vi_init_v2` 前 `CVI_VI_DisableChn(0,0)` |
| 3 | `HeightLSCnt=32` | OS04A10 内部 ISP 裁剪 16 行光学黑电平，crop 区域未预留 | crop end +16 行/列 (`os04a10_sensor_ctl.c`) |
| 4 | VTS 被 AE 改为 2432 | ISP bin (`cvi_sdr_bin.os04a10`) 硬编码 30fps AE 参数，AE 固件通过内部 I2C 写 VTS | post-init i2ctransfer VTS 覆写回 1216 |

### 涉及文件

| 文件 | 修改 |
|------|------|
| `os04a10_cmos_ex.h` | 添加 `OS04A10_MODE_1080P60_12BIT` 枚举 |
| `os04a10_cmos_param.h` | 添加 1080p60 模式参数表（HTS=1484, VTS=1216） |
| `os04a10_sensor_ctl.c` | 添加 `os04a10_linear_1080p60_12BIT_init()`，crop 修正 |
| `os04a10_cmos.c` | `cmos_set_image_mode()` 添加 1080p60 分支，`au32FL` 同步 |
| `sample_common_sensor.c` | `snsr_type_name[]` 数组修复 |
| `sample_comm.h` | 添加 1080p60 枚举值 |
| `maix_camera_mmf.cpp` | sensor 类型选择、FPS 钳位、VB 池修复、I2C VTS 覆写 |
| `sophgo_middleware.c/.h` | `mmf_add_vi_channel` v1 API 添加 `int fps` 参数 |

### 提交历史

```
1a7e9565 fix 1080p60: post-init VTS override via I2C
de91605f fix 1080p60: prevent AE from dropping fps (real root cause)
ef8b0097 fix os04a10 1080p60: VTS overwrite + crop area + VPSS rate
a5e5074f fix os04a10 1080p60: complete pipeline fix
d277c398 add OS04A10 1080p60 sensor mode + maixcam camera selection
```

---

## 二、WDR 模式（已验证）

### 目标
为 OS04A10 添加 2560×1440 WDR (Wide Dynamic Range) 模式，使用 DCG+VS 双曝光 staggered HDR，完整通过 VI→ISP→VPSS→应用层管道。

### WDR 传感器配置

| 参数 | 值 |
|------|-----|
| 分辨率 | 2688×1520 (sensor) → 2560×1440 (effective) |
| 帧率 | 30 fps (max) |
| 数据格式 | RAW10 (0x430B/C=0x0F/0xFC) |
| MIPI 模式 | 双 VC: DCG→VC0, VS→VC2 (0x4813=0x84) |
| HTS | 2972 (0x0B9C) |
| VTS | 1624 |
| MCLK | 24 MHz |
| MAC_CLK | 400M |
| ISP bin | `/mnt/cfg/param/cvi_wdr_bin.os04a10` |

### 已识别的软件 Bug 及修复

| # | 问题 | 修复 |
|---|------|------|
| 1 | WDR init HTS=732，参数表 u32HtsDef=2972 | 改为 0x0B9C=2972 |
| 2 | `enWDRMode[0]` 未同步，默认 `WDR_MODE_NONE` | 在 `_mmf_vi_init` 根据 sns_type 同步 |
| 3 | VI 双初始化导致 `Out of memory` | `DisableChn(0-3)` + `StopPipe` + `DestroyPipe` |
| 4 | ISP bin 未注册 `WDR_MODE_2To1_LINE` | 在 sns_type 选择时注册两个索引 |
| 5 | WDR FPS 未钳位 | 最大 30fps |
| 6 | `snsr_type_name[]` 数组断裂（与 1080p60 共用） | 已修复 |
| 7 | VB 池 3 block 不足 | `priv->vi_pool_num = 6` |
| 8 | `wdr_manu.update=1` 缺失（MANUAL 模式探索） | 已添加（但 MANUAL 模式未最终采用） |
| 9 | **VPSS 不接收帧** — `CVI_VPSS_GetChnFrame` 始终超时 | (见下方"VPSS 绑定修复"章节) |
| 10 | **第一次帧延迟 ~3 秒** — 管道需长时间稳定 | 增大 read() 超时到 3000ms |
| 11 | **帧率仅 6-8 FPS** — AE 将 VTS 从 1624 提高到 1948 | 设 `exp.stAuto.stExpTimeRange.u32Max=30ms`（缓解，未根治） |

### VPSS 绑定修复（根因分析）

`CVI_VPSS_GetChnFrame(0, 0, ...)` 超时的根因是 **VI 通道未启用**。具体调用链：

```
mmf_init_v2(false)          → SAMPLE_PLAT_VI_INIT → 创建管道+通道（线性模式）
SAMPLE_COMM_VI_IniToViCfg() → 准备 WDR 配置
WDR teardown:               → DisableChn(0-3) → 通道被禁用
                              StopPipe(0) + DestroyPipe(0) → 管道被销毁
mmf_vi_init_v2(...)         → 内部调用 CVI_VI_CreatePipe+StartPipe → 管道重新创建
                              [缺失] 未重新启用通道
CVI_VI_GetChnFrame(0,0,...) → 返回 0xC00E8040 = CVI_ERR_VI_FAILED_NOT_ENABLED
CVI_VPSS_GetChnFrame(0,0,.) → 返回 0xC006800E = CVI_ERR_VPSS_TIMEOUT
                                              （因绑定的 VI 通道无帧）
```

**三层修复，缺一不可：**

| 修复层 | 代码 | 作用 |
|--------|------|------|
| **VI 设备属性** | `CVI_VI_SetDevAttr(0, {enWDRMode=WDR_MODE_2To1_LINE})` | 第一次 init 将设备配置为线性模式；如果不更新为 WDR 模式，管道创建会使用错误的 DMA 配置 |
| **VI 通道启用** | `CVI_VI_EnableChn(0, 0)` | 拆卸禁用了通道；`mmf_vi_init_v2` 不重新启用，需要显式调用。返回 `0xC00E8041`（通道已启用）是可接受的 |
| **VI→VPSS 模式** | `CVI_SYS_SetVIVPSSMode({VI_OFFLINE_VPSS_OFFLINE})` | 确保 VPSS 使用标准离线模式（通过 DDR 绑定），与 `mmf_add_vi_channel_v2` 内部逻辑一致 |

**为什么之前尝试的 VI_VPSS_ONLINE 模式不起作用？**

`VI_OFFLINE_VPSS_ONLINE` + `VPSS_INPUT_ISP` 模式要求 VPSS 直接从 ISP 输出读取。但对于 `mmf_add_vi_channel_v2`（闭源库）创建的 VPSS 组，其内部默认使用 `VPSS_INPUT_MEM`。设置 ISP 输入模式可能与库内部逻辑冲突。回退到标准 `VI_OFFLINE_VPSS_OFFLINE` 与库的行为一致。

### 测试的三种 WDR 路径

#### 路径 1: VC 模式（选定方案）
- 传感器: `0x4813=0x84`（双 VC: DCG=VC0, VS=VC2）
- CSI: `HDR_EN=1`, `HDR_MODE=0`（VC 模式）
- SENSOR_MAC: 未启用（VC 模式不需要）
- devmem 覆写: `REG_04=0x00010000`, `VC_MAP=0x20`, `VS_GEN_MODE=2`
- **结果**: VI 管道成功运行（双通道帧流 7 FPS），但 VPSS 绑定未正确连接

#### 路径 2: MANUAL 模式（已放弃）
- 传感器: `0x4813=0x00`（共享 VC）
- CSI: `HDR_EN=1`, `CVI_MIPI_WDR_MODE_MANUAL`
- SENSOR_MAC: `HDR_EN=1`, `SHIFT=40`, `VSIZE=0x1FFF`
- **失败原因**: `CSI_ID_RM_ELSE/OB` 位过滤了共享 VC 流的数据，CSI 收到 0 数据

#### 路径 3: DCG 模式（待实现）
- 传感器内部 HDR 合成，输出单帧 RAW10
- 不需要 CSI HDR/VC_MAP/VS_GEN 特殊配置
- VI 以线性模式运行，ISP 以 `WDR_MODE_BUILT_IN` 处理
- **优点**: 完全绕过 CSI 和 VI 的 WDR 限制

### 当前状态（2026-07-11，最终）

```
传感器初始化 → ✅ MIPI PHY 时钟 HS → ✅ CSI HDR_EN=1 → ✅ VI 双通道接收 → ✅ ISP FSWDR 合并 → ✅
  → VI→VPSS 绑定 → ✅ (CVI_VPSS_GetChnFrame 成功，需 CVI_VI_SetDevAttr + CVI_VI_EnableChn)
  → 应用层帧读取 → ✅ (cam.read() 返回 2560x1440 NV21 帧)
```

**实测性能**（`cam.read()` 循环 3 帧的耗时，不含 Camera 构造函数）：

| 指标 | 值 |
|------|-----|
| 构造函数返回 | ~3s（~1s init + 2s VPSS 超时） |
| 第一帧延迟（构造后） | ~1.7s |
| 帧间隔 | 100-200ms（5-10 FPS） |
| 分辨率 | 2560×1440 |
| 像素格式 | NV21 |

**关键寄存器验证**:
- CSI_004 (HDR_CTRL): `0x000D0000` (HDR_EN=1, RM_ELSE=0, RM_OB=1)
- CSI_040 (STATUS): `0x00000000`（瞬时无数据—帧间消隐期）
- CSI_070 (VS_GEN): `0x00000F00` (mode=0, 单帧模式)
- CSI_018 (VC_MAP): `0x00003210` (identity 映射)
- MAC_040: `0x00000000` (SENSOR_MAC 未启用)
- PHY_CK: `0x00000001` (时钟 HS 模式)
- PHY_DT: `0x00000000` (数据线 Stop 状态)
- 传感器: `0x01` (streaming), HTS=`0x0b9c` (2972), VC=`0x84` (dual)

### 使用方式

```bash
# WDR 模式（已验证通过，可获取帧）
MAIX_WDR_MODE=2to1_line python3 -c "
from maix import camera
cam = camera.Camera(2560, 1440)
for i in range(3):
    img = cam.read()
    if img:
        print(f'frame {i}: {img.width()}x{img.height()}')
"
# 输出示例:
# frame 0: 2560x1440
# frame 1: 2560x1440
# frame 2: 2560x1440

# 线性 1080p60 模式（正常工作）
python3 -c "
from maix import camera
cam = camera.Camera(1920, 1080)
img = cam.read()
"

# 线性 1440p30 模式（正常工作）
python3 -c "
from maix import camera
cam = camera.Camera(2560, 1440)
img = cam.read()
"
```

### 遗留问题

| # | 问题 | 优先级 | 分析 |
|---|------|--------|------|
| 1 | **帧率 5-10 FPS（目标 30 FPS）** | **高** | 见下方"帧率分析"章节 |
| 2 | 第一次帧延迟 ~5s（构造函数~3s + 读取~2s） | 中 | 构造函数内的 VPSS 等待（2s 超时）可改短 |
| 3 | ISP bin sensor ID 不匹配：5440579→5440577 | 中 | 二进制 patch bin offset 815 |
| 4 | ISP bin MD5 不匹配：JSON 回退 | 低 | 二进制 patch bin offset 660 |
| 5 | 缺少 WDR 场景调优（BLC/AWB 来自 OV04A10 bin） | 低 | 需要 OS04A10 专用 WDR 校准 bin |

### 帧率分析

#### 帧率公式（来自 OS04A10 数据手册）

```
行时间 = HTS / SCLK
帧时间 = VTS × 行时间 = VTS × HTS / SCLK
帧率   = SCLK / (HTS × VTS)
```

其中 SCLK（系统时钟）由 PLL2 生成，最大 108 MHz；PCLK（像素时钟）由 PLL1 生成，最大 138 MHz。

#### 关键发现：WDR 模式下帧率减半的机制

数据手册第 1177 行明确说明：
> "For two exposure staggered HDR, the frame rate for each exposure frame will be 1/2."

这意味着在双曝光 staggered HDR 模式下，传感器内部以 **2 倍输出帧率** 运行。每个输出帧周期内，传感器完成两次内部帧扫描（一次长曝光，一次短曝光），输出单帧合并后的 HDR 数据。

#### 理论最大帧率计算

| 参数 | 线性 1440p30 | WDR 1440p30 WDR |
|------|-------------|-----------------|
| HTS | 1484 | **2972** (2× 适配双曝光像素交错) |
| VTS | 2432 | **1624** |
| HTS × VTS | 3,609,088 | **4,826,528** (1.34× 线性) |
| @108 MHz SCLK 帧率 | **30 fps** | **22.4 fps** |
| 目标帧率 | 30 fps | 30 fps |

**关键结论**：在最大 SCLK=108 MHz 时，WDR 模式的理论最大帧率为 **22.4 fps**，而非 30 fps。

要达到 30 fps，需满足：`SCLK = 30 × 2972 × 1624 = 144.8 MHz`。这超过了 PLL2 的最大输出 108 MHz。

数据手册 Table 3-2 列出 10-bit × 2 双曝光 HDR 在 2688×1520 下可达 **30 fps**，说明传感器设计上确实支持 30 fps。推测需要特定的 PLL 设置（可能使用不同的分频比或更高的 VCO 频率）。

#### 实际帧率 5-10 FPS 的原因

从 VI debug 看到的 6 fps 远低于理论值 22.4 fps，说明存在额外的管道瓶颈：

| 瓶颈 | 说明 | 影响 |
|------|------|------|
| **WDR2_VTS_0/1 未初始化 bug** | `cmos_get_sns_regs_info` 中 WDR 模式下 VTS 的 `u32Data` 留在 0。`os04a10_default_reg_init` 将 VTS=0 写入传感器（数据手册表明 VTS 无效值，传感器使用最小值） | 已修复。但传感器很可能忽略了 VTS=0（使用硬件默认 1624），因此该 bug 非帧率低的主因 |
| **ISP 处理带宽** | 2560×1440 双帧 ISP 处理（FSWDR 合并）计算量大 | 可能是主要瓶颈 |
| **VI DMA 带宽** | WDR 模式下 DMA 传输 2× 数据量 | 需验证 |
| **SCLK 时钟配置** | PLL2 当前设置 `divvp=144, prediv=2, divst=5` 不一定产生 108 MHz | 需用示波器或读取 PLL 锁定状态确认 |
| **AE 帧率控制** | AE 将 VTS 从 1624 提升到 1948（25fps 目标），但这不是主因（25→6 差距过大） | 可尝试 `AE_MODE_FIX_FRAME_RATE` |

#### 已实施的修复

| 修复 | 文件 | 行 | 说明 |
|------|------|---|------|
| **VTS 数据初始化** | `os04a10_cmos.c` | 1096-1101 | 为 `WDR2_VTS_0/1` 添加 `u32Data` 初始化（与 LINEAR 模式一致）。修复了 VTS=0 写入传感器的问题 |
| **AE 曝光范围限制** | `maix_camera_mmf.cpp` | 887 | `exp.stAuto.stExpTimeRange.u32Max = 30ms` — 限制 AE 使用过多曝光时间，减少 VTS 提升需求 |
| **VI 设备重配置** | `maix_camera_mmf.cpp` | 790 | `CVI_VI_SetDevAttr(WDR_MODE_2To1_LINE)` — 确保设备属性与 WDR 匹配 |
| **VI 通道启用** | `maix_camera_mmf.cpp` | 811 | `CVI_VI_EnableChn` — 管道重建后重新启用通道 |
| **VI_VPSS 模式** | `maix_camera_mmf.cpp` | 1020 | `VI_OFFLINE_VPSS_OFFLINE` — 标准离线 VPSS 绑定模式 |
| **Read 超时增加** | `maix_camera_mmf.cpp` | 1344 | WDR 模式从 100ms 增加到 3000ms |

#### 推荐的帧率修复方案

| 方案 | 说明 | 难度 | 预期效果 |
|------|------|------|---------|
| **1. 调整 PLL2 提高 SCLK** | 修改 `0x0322-0x0328` 寄存器值，提高系统时钟到 144.8 MHz（需确认传感器能否稳定工作于超频状态） | 中 | 30 fps |
| **2. 降低 HTS** | 当前 HTS=2972 可能过大。尝试降低到 ~2200（需确保不压缩像素数据） | 中 | 22-30 fps |
| **3. 使用 `AE_MODE_FIX_FRAME_RATE`** | 设置 AE 为固定帧率模式，防止 AE 改变 VTS | 低 | 缓解但非根因 |
| **4. ISP 管道优化** | 调整 ISP bin 参数以优化 WDR 处理性能 | 高 | 不确定 |
| **5. 示波器测量 SCLK** | 确认当前实际 SCLK 频率，确定可用余量 | 低（有设备时） | - |

#### 数据手册关键引用

> Table 3-2: Dual exposure HDR (DCG + VS), 10-bit × 2, 2688×1520 → **30 fps**
> 
> Section 3.4: `row_time = HTS / SCLK`
> 
> Section 3.7: Maximum SCLK = **108 MHz**, Maximum PCLK = **138 MHz**
> 
> Section 5.0: "For two exposure staggered HDR, the frame rate for each exposure frame will be 1/2"
> 
> Section 5.0: `Max_exposure_VS + Max_exposure_HCG/LCG < VTS - 10` (双曝光曝光时间限制)

### WDR 相关文件

| 文件 | 路径 |
|------|------|
| WDR init 函数 | `os04a10_sensor_ctl.c:938` (os04a10_wdr_1520p30_2to1_init) |
| MIPI RX 属性 | `os04a10_cmos.c:1315` (sensor_rx_attr) |
| WDR mode 设置 | `os04a10_cmos.c:981` (cmos_set_wdr_mode) |
| 参数表 | `os04a10_cmos_param.h:146` (OS04A10_MODE_1440P30_WDR) |
| 传感器枚举 | `os04a10_cmos_ex.h:71` (OS04A10_MODE_1440P30_WDR) |
| 应用层 WDR 选择 | `maix_camera_mmf.cpp:579,671` |
| enWDRMode 同步 | `maix_camera_mmf.cpp:743` |
| **VI 设备重配置** | **`maix_camera_mmf.cpp:790`** (CVI_VI_SetDevAttr WDR) |
| VI teardown | `maix_camera_mmf.cpp:785` |
| **VI 通道启用** | **`maix_camera_mmf.cpp:805`** (CVI_VI_EnableChn) |
| **VI_VPSS_MODE 设置** | **`maix_camera_mmf.cpp:1020`** |
| CSI 覆写 | `maix_camera_mmf.cpp:852` |
| AE 曝光范围限制 | `maix_camera_mmf.cpp:887` |
| WDR FPS 钳位 | `maix_camera_mmf.cpp:944` |
| **read() 超时增加** | **`maix_camera_mmf.cpp:1344`**（WDR 模式改为 3000ms） |
| ISP bin | `/mnt/cfg/param/cvi_wdr_bin.os04a10` |
| 编译部署 | `maixpy_wdr_build.md` |

### 修改提交

```
3d8ea0b9 fix WDR mode: VPSS frame delivery now working
e9e44ca7 wdr: skip VPSS first-frame wait for WDR mode, add VI pipe fallback
1e9e125a add os04a10 1440p30 WDR mode support (experimental)
```

---

## 三、关键发现

### VPSS 绑定问题根因

`CVI_VPSS_GetChnFrame` 超时的根因不是 VPSS 配置问题，而是 **VI 通道未启用**。

在 WDR 路径中：
1. `mmf_init_v2(false)` 用线性模式创建 VI 设备 → `CVI_VI_SetDevAttr(WDR_MODE_NONE)`
2. WDR 拆卸 `CVI_VI_DisableChn(0,0-3)` + `CVI_VI_DestroyPipe(0)` → 管道销毁但 VI 设备状态未更新
3. `mmf_vi_init_v2(...)` 用 WDR 配置重新创建管道 → 但不重新启用通道
4. `CVI_VI_GetChnFrame(0,0,...)` → `CVI_ERR_VI_FAILED_NOT_ENABLED` (0xC00E8040)
5. `CVI_VPSS_GetChnFrame(0,0,...)` → `CVI_ERR_VPSS_TIMEOUT` (0xC006800E)

**关键教训**：`mmf_vi_init_v2`（闭源库函数）在管道重建后不重新启用 VI 通道。必须显式调用 `CVI_VI_EnableChn(0, 0)`。此外，VI 设备属性必须先行更新为 WDR 模式，`CVI_VI_EnableChn` 才能正确工作。

### `mmf_vi_init_v2` 的行为分析

`mmf_vi_init_v2` 是闭源 `libmaixcam_lib.so` 中的函数，通过 `mmf_vi_init0()` 调度调用。它负责：
- 创建/重建 VI 管道（`CVI_VI_CreatePipe` + `CVI_VI_StartPipe`）
- 启动 ISP（加载 bin 文件）
- **不** 重新启用 VI 通道

WDR 拆卸-重建序列后，通道处于禁用状态。`mmf_vi_init_v2` 期望通道已在其外部启用。

### 关于闭源 `libmaixcam_lib.so` 的限制

`mmf_add_vi_channel_v2`、`mmf_vi_init_v2` 和 `mmf_init_v2` 都在闭源库中实现。所有 v2 函数都是 `sophgo_middleware.hpp` 中的内联包装器，通过 `MMF_FUNC_SET_PARAM` 分派号转发到 `mmf_*0()` 函数。这些函数位于 `/home/wlkeo/sg2002/MaixCDK/components/maixcam_lib/lib_maixcam/libmaixcam_lib.so` 中。

`CONFIG_MAIXCAM_LIB_COMPILE_FROM_SOURCE` cmake 选项存在但源目录 `maixcdk_maixcam_lib/` 在仓库中不存在。

### 关于 VI 错误码 0xC00E8040 的含义

| 错误码 | 宏 | 含义 |
|--------|-----|------|
| `0xC00E8040` | `CVI_ERR_VI_FAILED_NOT_ENABLED` | VI 通道未启用 |
| `0xC00E8041` | `CVI_ERR_VI_FAILED_NOT_DISABLED` | VI 通道已启用（启用二次调用） |
| `0xC006800E` | `CVI_ERR_VPSS_TIMEOUT` | VPSS GetChnFrame 超时（3s 内无帧） |

### 关于"CSI 无法锁定帧时序"的再评估

之前分析文档（2026-06）声称 SG2002 CSI bit19=1 (VC帧检测) 且 bit20=0 (帧锁定失败)。
TRM 和驱动源码分析证实 **SG2002 CSI 没有 bit19/bit20 状态位**。这些位来自其他平台。

经过修复后的 WDR 模式测试证明：
- CSI HDR_EN=1 正常生效
- VI 两个 ISP 通道都收到帧（VISofCh0Cnt=8, VISofCh1Cnt=8）
- ISP FSWDR 成功合并帧（VIPostCnt=7）
- **所有帧都能通过 VPSS 读取（修正后）**

**"无法锁定时序"的结论不成立**。实际的硬件管道在修复后工作正常。

### 关于寄存器数据污染的教训

早期测试的 devmem 写入在模块卸载后仍然保留在硬件寄存器中。跨测试比较需要在重启后进行洁净测试。

### OS04A10 vs OS04C10 sensor ID

- OS04A10_ID = `0x530441` = 5440577
- OS04C10_ID = `0x530443` = 5440579
- SDR 和 WDR ISP bin 都包含 sensorName=`5440579`（OS04C10 的 ID）
- sensor name 检查是软检查（返回成功），所以 bin 仍能被加载

---

## 四、编译与部署

### 编译命令

```bash
cd ~/sg2002/MaixPy/build
cmake .. -DCONFIG_MAIXCAM_PRO=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make sophgo-middleware -j$(nproc)   # libsophgo-middleware.so
make maix -j$(nproc)                # libmaix.so
```

### 部署命令

```bash
# libsophgo-middleware.so (传感器驱动修改)
scp build/sophgo-middleware/libsophgo-middleware.so root@<IP>:/usr/lib/python3.11/site-packages/maix/dl_lib/

# _maix.so (camera 初始化修改)
scp build/maix/libmaix.so root@<IP>:/usr/lib/python3.11/site-packages/maix/_maix.so

# WDR ISP bin
scp LicheeRV-Nano-Build-main/isp_tuning/sg200x/src/ov_os04a10/ov_os04a10_wdr.bin root@<IP>:/mnt/cfg/param/cvi_wdr_bin.os04a10
```

### 调试命令速查

```bash
# CSI 寄存器
devmem 0x0A0C2404 32    # HDR_CTRL (bit16=HDR_EN, bit17=HDR_MODE)
devmem 0x0A0C2440 32    # CSI_STATUS (ECC, DECODE_FORMAT)
devmem 0x0A0C2460 32    # INTR_STATUS
devmem 0x0A0C2470 32    # VS_GEN
devmem 0x0A0C2418 32    # VC_MAP

# SENSOR_MAC
devmem 0x0A0C2040 32    # HDR_CTRL
devmem 0x0A0C2044 32    # HDR_PARAM (SHIFT+VSIZE)
devmem 0x0A0C20C4 32    # DBG_HTOTAL (实测 H 总长)
devmem 0x0A0C20C8 32    # DBG_VTOTAL (实测 V 总长)

# PHY
devmem 0x0A0D0390 32    # CK_STATE (bit0=HS)
devmem 0x0A0D0394 32    # DATA_STATE (bits 2:0 per lane)

# 传感器 I2C
i2ctransfer -y -f 4 w2@0x36 0x01 0x00 r1       # streaming
i2ctransfer -y -f 4 w2@0x36 0x30 0x0a r3       # chip ID
i2ctransfer -y -f 4 w2@0x36 0x38 0x0c r2       # HTS
i2ctransfer -y -f 4 w2@0x36 0x38 0x0e r2       # VTS
i2ctransfer -y -f 4 w2@0x36 0x48 0x13 r1       # VC mapping

# VI 状态
cat /proc/cvitek/vi_dbg
cat /proc/cvitek/vi
cat /proc/cvitek/vb

# 全寄存器对比（LINEAR vs WDR），需重启后洁净运行
sh /tmp/clean_capture.sh linear
sh /tmp/clean_capture.sh wdr
```

---

## 五、1080p WDR 模式实现尝试（2026-07-12）

### 目标

将 WDR 模式从 1440p（2560×1440）降到 1080p（1920×1080），减少 ISP FSWDR 处理负载约 43%，预期帧率从 7-8fps 提升到 15-20fps。

### 已完成的工作

#### 5.1 传感器驱动层（已实现并验证通过）

| 文件 | 修改 |
|------|------|
| `os04a10_cmos_ex.h` | 添加 `OS04A10_MODE_1080P60_WDR` 枚举（值=4），更新 `OS04A10_MODE_NUM` |
| `os04a10_cmos_param.h` | 添加 1080p60 WDR 参数表：HTS=2972, VTS=1216, stSnsSize={1920,1080} |
| `os04a10_sensor_ctl.c` | 新建 `os04a10_wdr_1080p60_2to1_init()`（合并 1080p 裁剪 + WDR 模拟/VC 配置）；更新 `os04a10_init()` 分发 |
| `os04a10_cmos.c` | `cmos_set_image_mode()` 添加 WDR_MODE_2To1_LINE + 1080P 分支；`cmos_set_wdr_mode()` 添加 1080p 线性/WDR 切换；`cmos_set_image_mode()` 的 WDR_MODE_NONE 路径添加 1080p 回落选择（解决 cmos_set_image_mode 在 cmos_set_wdr_mode 之前调用的问题） |

**传感器寄存器验证结果（通过 I2C 读回）：**
```
OUT_W: 0x0780 = 1920
OUT_H: 0x0438 = 1080
HTS:   0x0B9C = 2972
VTS:   0x04C0 = 1216
CROP:  (384,220) → (2319,1315)
VC:    0x84（双 VC: DCG=VC0, VS=VC2）
STRM:  0x01（streaming）
```
✅ 传感器 1080p WDR 初始化完全正确。

#### 5.2 应用层管道（已实现）

| 文件 | 修改 |
|------|------|
| `sample_comm.h` | 添加 `OV_OS04A10_MIPI_4M_1080P60_10BIT_WDR2TO1` 枚举 |
| `sample_common_sensor.c` | 枚举→`PIC_1080P` 映射、sensor obj 注册、字符串名称表 |
| `maix_camera_mmf.cpp` | `_mmf_vi_init` 中根据分辨率选择 WDR 传感器类型（1080p→新类型，1440p→旧类型）；更新所有 WDR 管道检查（`priv->sns_type` 比较）包含两种类型；`WDR_MODE_2To1_LINE` bin 路径根据需要选择 `cvi_wdr_bin_1080p.os04a10` 或 `cvi_wdr_bin.os04a10` |

#### 5.3 ISP bin 修补（已完成）

`cvi_wdr_bin.os04a10` 的 ISP0 段中 `ISP_PUB_ATTR_S` 包含硬编码分辨率：

```c
stSnsSize = {2688, 1520}   → 修补为 {1920, 1080}
stWndRect = {0, 0, 2688, 1520} → 修补为 {0, 0, 1920, 1080}
```

修补后的 bin 保存在 `cvi_wdr_bin_1080p.os04a10`，MD5 不匹配但加载器将该错误视为非致命（`check_bin_file_validity` 返回 `CVI_BIN_DATA_ERR` 但主加载器只对 `CVI_BIN_FILE_ERROR` 跳转错误处理），数据仍被加载到 ISP。

#### 5.4 MIPI RX 重配置（已实现）

在 `_mmf_vi_init` 的 WDR 初始化完成后（`CVI_VI_EnableChn` 之后），添加：

```cpp
const ISP_SNS_OBJ_S *pstSnsObj = (ISP_SNS_OBJ_S *)SAMPLE_COMM_ISP_GetSnsObj(0);
if (pstSnsObj) {
    SNS_COMBO_DEV_ATTR_S stRxAttr;
    pstSnsObj->pfnGetRxAttr(0, &stRxAttr);
    CVI_MIPI_SetMipiAttr(0, (CVI_VOID *)&stRxAttr);
}
```

这使用当前传感器模式（u8ImgMode=4=1080P60_WDR）下的 `sensor_rx_attr` 返回值（img_size={1920,1080}）重新配置 CSI 寄存器。日志确认 `MIPI RX attr refreshed` 被打印。

#### 5.5 `pool_num` 修复（已实现）

`mmf_add_vi_channel_v2` 末尾的 `pool_num` 参数从局部变量（在 `_mmf_vi_init` 设置 `priv->sns_type` 之前初始化为 3）改为 `priv->vi_pool_num`（在 `_mmf_vi_init` 内部设置为 WDR 模式的 6）。

### 当前状态

```
open() 成功 → ✅
VI 接收帧 (VIDevFPS=10) → ✅
ISP 处理帧 (VIPostCnt increment) → ✅
VI DMA 活跃 (VIWdma0 非全 idle) → ✅
MIPI RX 已刷新 → ✅
VPSS GetChnFrame → ❌ 始终超时
cam.read() → ❌ "camera read timeout"
```

### 逆向分析：`libmaixcam_lib.so`（闭源库）

#### 5.6.1 二进制概要

| 属性 | 值 |
|------|-----|
| 路径 | `components/maixcam_lib/lib_maixcam/libmaixcam_lib.so` |
| 架构 | RISC-V 64-bit, LP64, double-float ABI, stripped |
| 大小 | 905 KB |
| 573 个 .dynsym 导出符号 |

#### 5.6.2 `mmf_add_vi_channel_v2` 的调用链

`sophgo_middleware.hpp:264` 中的内联包装器：
```cpp
static inline int mmf_add_vi_channel_v2(int ch, int width, int height, int format,
    int fps, int depth, int mirror, int vflip, int fit, int pool_num) {
    return mmf_add_vi_channel0(MMF_FUNC_SET_PARAM(0, 10),
        ch, width, height, format, fps, depth, mirror, vflip, fit, pool_num);
}
```

`mmf_add_vi_channel0` (0x21A94) → 提取可变参数 → 调用 `mmf_add_vi_channel` (0x1EE00) → 调用 `mmf_set_vi_vflip` (0x1EBCC，实际核心实现)。

#### 5.6.3 `mmf_set_vi_vflip` 的反汇编伪代码

```
mmf_set_vi_vflip(vi_pipe, width, height, chn, framerate, encode, hmirror, vflip, extra):
    if (!init_done) return -1
    if (width <= 0 || height <= 0) return -1
    if (mmf_vi_chn_is_open(vi_pipe)) return -1

    CVI_VPSS_DisableChn(0, vi_pipe)          // 关闭旧 VPSS 通道
    
    internal_vpss_setup(vi_pipe, width, height, ...):
        CVI_VPSS_GetGrpAttr(0, &grp_attr)    // 获取组属性
        grp_attr.MaxWidth = width             // ← 设置为调用者传入的宽度
        grp_attr.MaxHeight = height           // ← 设置为调用者传入的高度
        CVI_VPSS_SetChnCrop(0, vi_pipe, &crop)
        CVI_VPSS_SetChnAttr(0, vi_pipe, &chn_attr)
        CVI_VPSS_EnableChn(0, vi_pipe)

    SAMPLE_COMM_VI_Bind_VPSS(vi_pipe, chn, 0)  // 绑定 VI→VPSS
    
    create_vpss_pool("VPSS_Group_%d", width, chn, pool_num)
    CVI_VPSS_AttachVbPool(0, vi_pipe, pool_id)  // 附加 VB 池
```

**关键发现**：VPSS 组的 `MaxWidth/MaxHeight` 被设置为**调用者传入的 `width/height`**（即 Camera 构造函数的 `_width/_height`），而非传感器原生分辨率。这与 v1 开源实现（`sophgo_middleware.c`）不同——v1 使用传感器原生大小作为 VPSS 组输入，使用用户请求大小作为 VPSS 通道输出。

#### 5.6.4 池绑定分析

`mmf_vi_init_v2` 创建 VI 的 VB 池（`priv->vi_pool_num` 个缓冲区），而 `mmf_add_vi_channel_v2` 创建**独立的** VPSS VB 池并通过 `CVI_VPSS_AttachVbPool` 附加。`SAMPLE_COMM_VI_Bind_VPSS` 需要在系统层面绑定两个池。

对于 WDR 模式，如果池大小不匹配（VI 池用 6 个缓冲区，VPSS 池用 3 个），绑定可能会静默失败。`pool_num` 修复解决了这个问题，但 VPSS 仍然超时。

### 注意事项

1. **CSI/MIPI RX 在 VC 模式下不验证帧大小**。`cif_hdr_csi_enable` 仅设置 `HDR_EN=1`、`HDR_MODE=0`，不设置预期宽度/高度。CSI 使用 MIPI 数据包头部字计数确定每行长度——传感器输出 1920x1080 时，CSI 接收 1920x1080 帧的正确数据。

2. **SENSOR_MAC 寄存器（0x0A0C2040-0x0A0C2044）在 VC 模式下保持为 0**。`cif_hdr_manual_config` 仅在 `CVI_MIPI_WDR_MODE_MANUAL` 模式下调用，`CVI_MIPI_WDR_MODE_VC` 模式会跳过它。`MAC_040=0` 是预期行为。

3. **VPSS 组 0 硬编码**：逆向确认所有 VPSS 操作（创建、设置、绑定）都使用组 0。这是闭源库内部的硬编码限制。

4. **`mmf_add_vi_channel_v2` 与 `mmf_add_vi_channel` 的行为不同**：v2 将 VPSS 组的 `MaxWidth/MaxHeight` 设置为用户请求的分辨率，而 v1 使用传感器原生分辨率。这意味着在 v2 API 中，VPSS 组输入大小直接匹配 VPSS 通道输出大小（无缩放）。在 WDR 模式下，VI 输出的分辨率需要与 VPSS 组输入分辨率匹配。

5. **`pool_num` 始终为 6 对 WDR 模式至关重要**。闭源库的 `create_vpss_pool` 使用 `pool_num` 参数分配缓冲区数量。WDR 模式需要更多缓冲区来处理双帧数据。`pool_num` 未正确设置（由于 `priv->sns_type` 在 `Camera::open()` 的池大小检查点尚未设置）导致 VPSS 池只有 3 个缓冲区。

6. **ISP bin 包含硬编码分辨率**。`cvi_wdr_bin.os04a10` 的 ISP0 段中的 `ISP_PUB_ATTR_S` 包含 `stSnsSize={2688,1520}` 和 `stWndRect={0,0,2688,1520}`。当 ISP 加载修补后的 bin 时，`isp_set_paramstruct` 调用 `CVI_ISP_SetPubAttr`，该调用将 ISP 内部处理分辨率覆盖为 bin 中的值。如果 bin 中的分辨率（1920x1080）与 VI 管道分辨率（1920x1080）匹配，则 ISP 应正确工作。

### 关于 `libmaixcam_lib.so` 闭源库的发现

- `mmf_add_vi_channel_v2`、`mmf_vi_init_v2`、`mmf_init_v2` 都是通过 `MMF_FUNC_SET_PARAM` 分派号的包装器
- 所有 VPSS 操作使用组 0（硬编码）
- 库中没有 WDR/HDR 特定代码——WDR 模式完全由传感器驱动和 ISP bin 处理
- 库依赖 `libsophgo-middleware.so`、`libsys.so`、`libisp.so` 等外部 SDK 库
- 库导入的 SDK API：`CVI_VPSS_CreateGrp/DestroyGrp/SetChnAttr/GetChnAttr/EnableChn/DisableChn/StartGrp/StopGrp`、`CVI_VI_SetDevNum/AttachVbPool/DetachVbPool`、`SAMPLE_COMM_VI_Bind_VPSS`
- 开源 v1 API（`sophgo_middleware.c`）显示 `SAMPLE_COMM_VI_Bind_VPSS(Dev, Chn, VPSSGrp)` 是绑定调用的核心
- v1 与 v2 的关键区别：v1 的 `_mmf_vpss_init` 将传感器原生尺寸设置为 VPSS 组输入大小，用户尺寸设置为通道输出大小；v2 将用户尺寸同时用于组和通道（通过 `mmf_add_vi_channel_v2` 参数）

### 关于 `pool_num` 时序问题的补充分析

在 `Camera::open()` 中：

```
#990: int pool_num = 3;                           // 初始值为 3
#996: if (priv->sns_type == WDR) pool_num = 6;    // priv->sns_type 未设置！→ 跳过
#999: priv->vi_pool_num = pool_num;                // = 3
...
#1033:  _mmf_vi_init(...);
        // 内部设置 priv->vi_pool_num = 6
        // 使用 priv->vi_pool_num = 6 调用 mmf_vi_init_v2
...
#1045:  mmf_add_vi_channel_v2(..., pool_num)       // pool_num 仍为 3！
```

修复：将 `pool_num` 替换为 `priv->vi_pool_num`。

---

## 六、1440p WDR 回归与 VTS 初始化修复（2026-07-12）

### 问题描述

`3d8ea0b9` 提交（完整 VPSS 修复）后 1440p WDR 工作正常。后续提交 `ab985c41`（VTS 初始化修复）导致 1440p WDR 无法工作：VI DMA 停滞，VPSS 收不到帧，`cam.read()` 超时。

### 精确根因定位

通过受控对比实验（相同构建系统、相同摄像头库、仅传感器库不同）确认：

| 构建来源 | VTS 初始化 | WDR 结果 |
|---------|-----------|---------|
| `3d8ea0b9`（无 VTS 修复） | `u32Data` 未设置（memset 为 0） | ✅ 工作 |
| `ab985c41`（有 VTS 修复） | `u32Data` = 1624 | ❌ 失败 |

**差异代码（`os04a10_cmos.c` 第 1096-1101 行）：**

```c
// 3d8ea0b9（工作）—— u32Data 未初始化，保持为 0
pstI2c_data[WDR2_VTS_0].u32RegAddr = OS04A10_VTS_ADDR;
pstI2c_data[WDR2_VTS_1].u32RegAddr = OS04A10_VTS_ADDR + 1;

// ab985c41（失效）—— u32Data 显式设置为 1624
pstI2c_data[WDR2_VTS_0].u32RegAddr = OS04A10_VTS_ADDR;
pstI2c_data[WDR2_VTS_0].u32Data = (1624 >> 8) & 0xFF;  // = 0x06
pstI2c_data[WDR2_VTS_1].u32RegAddr = OS04A10_VTS_ADDR + 1;
pstI2c_data[WDR2_VTS_1].u32Data = 1624 & 0xFF;           // = 0x58
```

### 执行路径分析

VTS 寄存器（0x380E/0x380F）在传感器初始化期间被写入两次：

1. **`os04a10_default_reg_init()`**（`os04a10_sensor_ctl.c:160-174`）—— 在 `os04a10_wdr_1520p30_2to1_init()` 函数末尾、`0x0100=0x01`（开启流）之前调用。遍历 I2C 寄存器表（索引 1 到 `u32RegNum-3`），将每个条目的 `u32RegAddr` 和 `u32Data` 直接写入传感器。对于 VTS 条目：
   - 修复前：写入 VTS=0（`u32Data` 来自 memset，默认为 0）
   - 修复后：写入 VTS=1624（`u32Data` 被显式设置）

2. **`cmos_fps_set()`**（`os04a10_cmos.c:277-278`）—— 在 AE 启动后由 ISP 线程调用，通过 `bvblankUpdate` 机制在帧消隐期更新 VTS。无论第 1 步写入的值如何，此步骤都会将 VTS 更新为根据 FPS 计算的 `u32VMAX`。

### 关键时序

```
os04a10_wdr_1520p30_2to1_init():
  行 943:  0x0103 = 0x01      ← 软复位（传感器停止输出，所有寄存器复位）
  行 944-1245: 配置 WDR 模式所需的数百个寄存器
  行 1246: os04a10_default_reg_init()  ← 直接写入寄存器表（含 VTS）
  行 1247: 0x0100 = 0x01      ← 开启流输出

cmos_fps_set()（AE 线程，稍后调用）:
  行 277-278: 更新 VTS 为 u32VMAX  ← 通过 blanking 更新写入传感器
```

### 疑点

VTS=1624 是合法值，VTS=0 是非法值（数据手册：`l_exp_max = VTS - 8`，VTS=0 时为负值）。两者都在开启流之前写入，传感器应处于静默状态，值本身不应影响后续行为。但实证表明 VTS=1624 导致 WDR 失败。

**可能的解释**：
1. `os04a10_default_reg_init` 直接写入寄存器（不通过 group hold），而数据手册明确指出直接 SCCB 写入"不保证在帧边界生效"。对于 WDR 模式，VTS 时序寄存器可能需要通过 group hold 机制写入才能正确配置传感器的双曝光交错时序。
2. WDR 初始化序列的具体寄存器写入顺序属于 OmniVision NDA 内容，公开数据手册中不包含。`default_reg_init` 的通用写入可能与 WDR 专用初始化序列存在隐式依赖冲突。
3. VTS=0 被传感器内部忽略（使用默认时序），而 VTS=1624 被接受并立即生效，但此时 WDR 时序尚未完全锁定，导致内部状态机错位。

### 修复

从 `cmos_get_sns_regs_info()` 中移除 WDR2_VTS_0/1 的 `u32Data` 显式赋值，恢复旧行为：

```c
// 修复后（恢复旧行为）
pstI2c_data[WDR2_VTS_0].u32RegAddr = OS04A10_VTS_ADDR;
pstI2c_data[WDR2_VTS_1].u32RegAddr = OS04A10_VTS_ADDR + 1;
```

`os04a10_default_reg_init` 写入 VTS=0（传感器忽略），VTS 最终由 `cmos_fps_set` 通过 blanking 更新路径正确设置。

### 教训

1. **传感器寄存器初始化不能假设"值正确就一定没问题"**。寄存器写入的时序、路径（直接 SCCB vs group hold）、以及与其他寄存器的相对顺序都可能影响传感器行为。即使在"开启流之前"的静默状态下，某些寄存器的写入也可能触发内部状态机的变化。

2. **VTS 等关键时序寄存器应通过 blanking 更新路径（`bvblankUpdate`/`cmos_fps_set`）设置**，而非在 `default_reg_init` 中直接写入。`default_reg_init` 是通用寄存器初始化函数，不适合处理 WDR 模式下有特殊时序要求的寄存器。

3. **闭源传感器的 WDR 初始化序列可能有未文档化的依赖关系**。公开数据手册不包含 WDR 模式的详细初始化时序要求，修改初始化代码时需要格外谨慎。

4. **`cmos_fps_set()` 中已有独立的 VTS 更新逻辑**（`os04a10_cmos.c:277-278`），它在 AE 启动后通过 blanking 路径正确设置 VTS。`cmos_get_sns_regs_info` 中的 VTS 初始化是冗余的。

5. **回归测试必须使用相同的构建环境**。本次分析中，旧构建系统（373KB 库）和新构建系统（528KB 库）产生了不同大小的二进制文件，可能导致不同的编译器优化行为。精确对比必须控制构建环境变量。

---

## 七、MaixCDK 构建系统恢复记录（2026-07-12）

### 背景

构建目录 `/home/wlkeo/sg2002/MaixPy/build` 被删除后，MaixCDK 构建系统无法直接通过 `cmake` 重建，因为：
1. `global_config.cmake` / `global_config.h` / `global_config_platform.h` 由 `menuconfig` 生成
2. `toolchain_config.cmake` 由平台配置生成
3. 组件验证需要正确的 `PLATFORM` 变量
4. 交互式 `menuconfig` 需要终端（无法在非交互式 shell 中运行）

### 恢复步骤

#### 1. 非交互式生成 config 文件

```bash
PLATFORM=maixcam BUILD_TYPE=Release python3 \
  /home/wlkeo/sg2002/MaixCDK/tools/kconfig/genconfig.py \
  --kconfig /home/wlkeo/sg2002/MaixCDK/Kconfig \
  --defaults /tmp/config_maixcam_defaults.mk \
  --menuconfig False \
  --env "PLATFORM=maixcam" \
  --env "SDK_PATH=/home/wlkeo/sg2002/MaixCDK" \
  --env "PROJECT_PATH=/home/wlkeo/sg2002/MaixPy" \
  --env "BUILD_TYPE=Release" \
  --output cmake /home/wlkeo/sg2002/MaixPy/build/config/global_config.cmake \
  --output header /home/wlkeo/sg2002/MaixPy/build/config/global_config.h \
  --output makefile /home/wlkeo/sg2002/MaixPy/build/config/global_config.mk
```

#### 2. 手动创建缺失文件

```bash
# global_config_platform.h
cat > /home/wlkeo/sg2002/MaixPy/build/config/global_config_platform.h << 'EOF'
#ifndef __GLOBAL_CONFIG_PLATFORM_H__
#define __GLOBAL_CONFIG_PLATFORM_H__
#define PLATFORM_MAIXCAM 1
#define PLATFORM "maixcam"
#endif
EOF

# toolchain_config.cmake（toolchain 路径来自 platforms/maixcam.yaml）
cat > /home/wlkeo/sg2002/MaixPy/build/config/toolchain_config.cmake << 'EOF'
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)
set(CONFIG_TOOLCHAIN_PATH "/home/wlkeo/sg2002/host-tools/gcc/riscv64-linux-musl-x86_64/bin")
set(CONFIG_TOOLCHAIN_PREFIX "riscv64-unknown-linux-musl-")
set(CMAKE_C_COMPILER "${CONFIG_TOOLCHAIN_PATH}/${CONFIG_TOOLCHAIN_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${CONFIG_TOOLCHAIN_PATH}/${CONFIG_TOOLCHAIN_PREFIX}g++")
set(CMAKE_C_FLAGS "-mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d")
set(CMAKE_CXX_FLAGS "-mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d")
EOF
```

#### 3. 修复 cvi_tpu 组件

cvi_tpu 需要下载的预编译库（在 `dl/pkgs/cvi_tpu` 目录下），如果缺失则 cmake 报错。临时修复：注释掉 `FATAL_ERROR`，改为 `WARNING`。

#### 4. 运行 cmake

```bash
cd /home/wlkeo/sg2002/MaixPy/build && rm -f CMakeCache.txt CMakeFiles -rf
PLATFORM=maixcam cmake .. \
  -DSDK_PATH=/home/wlkeo/sg2002/MaixCDK \
  -DPROJECT_ID=maixpy \
  -DPLATFORM=maixcam \
  -DCONFIG_MAIXCAM_PRO=ON \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

#### 5. 添加 PROJECT_ID 编译定义

`basic` 组件的 `maix_app.cpp` 使用 `PROJECT_ID` 宏，需要在 `compile/compile_flags.cmake` 中添加：

```cmake
add_definitions(-DPROJECT_ID="maixpy")
```

#### 6. 编译

```bash
make sophgo-middleware -j$(nproc)   # 传感器驱动库
make maix -j$(nproc)               # 摄像头库（需解决 freetype 等依赖）
```

### 已知限制

- `maix` 库编译依赖 freetype 等预编译库，如果这些库的 cmake 版本要求与当前 cmake 不兼容，编译会失败。
- 仅 `sophgo-middleware`（传感器驱动）可以独立编译，`maix`（摄像头管线）需要完整的 MaixCDK 构建环境。
- 测试 1080p/1440p WDR 时，可以只部署 `libsophgo-middleware.so`，保留 `libmaix.so` 不变。

---

## 八、关于闭源 `libmaixcam_lib.so` 的逆向分析

### 二进制概要

| 属性 | 值 |
|------|-----|
| 路径 | `components/maixcam_lib/lib_maixcam/libmaixcam_lib.so` |
| 架构 | RISC-V 64-bit, LP64, double-float ABI, stripped |
| 大小 | 905 KB |
| 导出符号 | 573 个 .dynsym |

### 关键函数

| 函数 | 地址 | 作用 |
|------|------|------|
| `mmf_add_vi_channel_v2` 包装器 | 内联（`sophgo_middleware.hpp:264`） | 通过 `MMF_FUNC_SET_PARAM(0,10)` 分派 |
| `mmf_add_vi_channel0` | 0x21A94 | 提取可变参数，调用 `mmf_add_vi_channel` |
| `mmf_add_vi_channel` | 0x1EE00 | 调用 `mmf_set_vi_vflip` |
| `mmf_set_vi_vflip` | 0x1EBCC | 创建 VPSS 组/通道、绑定 VI→VPSS、创建 VPSS 池 |
| `mmf_vi_init_v2` 包装器 | 内联 | 通过 `MMF_FUNC_SET_PARAM(0,3)` 分派 |
| `mmf_init_v2` 包装器 | 内联 | 通过 `MMF_FUNC_SET_PARAM(0,2)` 分派 |

### VPSS 组/通道行为

- VPSS 组 0 硬编码（所有操作使用组 0）
- `MaxWidth/MaxHeight` 设置为用户请求的分辨率（非传感器原生分辨率）
- 与 v1 开源实现不同：v1 将传感器原生尺寸设为组输入，用户尺寸设为通道输出
- VPSS 池独立于 VI 池，通过 `CVI_VPSS_AttachVbPool` 附加

### WDR 模式相关发现

- 库中没有 WDR/HDR 特定代码
- WDR 模式完全由传感器驱动和 ISP bin 处理
- `mmf_vi_init_v2` 内部使能 VI 通道（`CVI_VI_EnableChn` 返回 0xC00E8041 表示已使能）
- `mmf_vi_init_v2` 不重新配置 MIPI RX（需要外部调用 `CVI_MIPI_SetMipiAttr`）

### 导入的 SDK API

```
CVI_VPSS_CreateGrp / DestroyGrp / SetChnAttr / GetChnAttr
CVI_VPSS_EnableChn / DisableChn / StartGrp / StopGrp
CVI_VI_SetDevNum / AttachVbPool / DetachVbPool
SAMPLE_COMM_VI_Bind_VPSS
```

---

## 九、OS04A10 VTS 寄存器（0x380E/0x380F）技术参考

### 寄存器定义

| 寄存器 | 名称 | 位定义 |
|--------|------|--------|
| 0x380E | TIMING_CTRL_14 | `{Frame_Length[15:8]}` — 帧总行数高字节 |
| 0x380F | TIMING_CTRL_15 | `{Frame_Length[7:0]}` — 帧总行数低字节 |

### 帧率公式

```
帧率 = SCLK / (HTS × VTS)
```

其中 SCLK 最大 108 MHz，PCLK 最大 138 MHz。

### 双曝光模式下的曝光约束

```
l_exp_max = VTS - 8                    // 长帧最大曝光行数
m_exp_max = VTS - l_exp_max - 2        // 短帧最大曝光行数
Max_exposure_VS + Max_exposure_HCG/LCG < VTS - 10
```

### Group Hold 机制（0x3208）

0x3208 是 Group Access Control 寄存器：
- `Bits[7:4]`: 操作类型
  - `0x0` = hold start（开始记录组寄存器写入）
  - `0x1` = hold end（结束记录）
  - `0xA` = delay manual launch（延迟手动启动）
  - `0xE` = quick manual launch（立即手动启动）
- `Bits[3:0]`: 组号（0-5）

数据手册明确指出：group hold 功能"allows configuring of many of the sensor's parameters in a single instance, something that **cannot be applied or guaranteed when using direct SCCB register writes**"。

### WDR 初始化序列

WDR 模式的详细寄存器写入序列属于 OmniVision NDA 内容，不在公开数据手册中。当前使用的 `os04a10_wdr_1520p30_2to1_init()` 函数基于供应商参考代码。

---

## 十、开源 VPSS Bypass 实现（2026-07-17）

### 背景

闭源库 `libmaixcam_lib.so` 中的 `mmf_init_v2` + `mmf_vi_init_v2` + `mmf_add_vi_channel_v2` 存在多重问题：
- `mmf_init_v2(false)` + `mmf_vi_init_v2()` 双初始化耗尽 3-block VB 池 → SIGSEGV
- 闭源库无法修改，调试困难
- 1080p60 + WDR 模式需要灵活控制 VI→VPSS 管线

目标：用开源 `SAMPLE_PLAT_VI_INIT` + `SAMPLE_PLAT_VPSS_INIT` 完全替代闭库调用链。

### 架构设计

```
Camera::open()
  ├─ _mmf_vi_init()          ← 替代 mmf_init_v2 + mmf_vi_init_v2
  │   ├─ 自定义 VB 池 (6块)
  │   ├─ CVI_SYS_SetVIVPSSMode(VI_OFFLINE_VPSS_OFFLINE)
  │   └─ SAMPLE_PLAT_VI_INIT + CVI_VI_EnableChn
  │
  ├─ SAMPLE_PLAT_VPSS_INIT   ← 替代 mmf_add_vi_channel_v2 的 VPSS 创建部分
  ├─ VPSS 通道重启 + 池绑定  ← 替代 mmf_add_vi_channel_v2 的池绑定部分
  ├─ SAMPLE_COMM_VI_Bind_VPSS
  └─ CVI_VPSS_GetChnFrame(首帧验证)
```

### 遇到的问题及修复

#### 问题 1: VB 池耗尽导致 SIGSEGV

`mmf_init_v2(false)` 先创建一个 3-block 公共池，`mmf_vi_init_v2()` 再次初始化时会耗尽所有块。第二个 `CVI_VI_EnableChn` 返回 `CVI_ERR_VB_NOBUF`，触发 `_SAMPLE_PLAT_ERR_Exit` → 模块卸载 → 野指针 → SIGSEGV。

**修复**: 用 `SAMPLE_COMM_SYS_Init` 创建自定义 VB 池，跳过 `mmf_init_v2`：

```cpp
// 自定义 VB 池 (6 块, 传感器原生大小)
VB_CONFIG_S vb;
memset(&vb, 0, sizeof(vb));
vb.u32MaxPoolCnt = 1;
vb.astCommPool[0].u32BlkSize = COMMON_GetPicBufferSize(
    sys_size.u32Width, sys_size.u32Height,
    SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8,
    COMPRESS_MODE_NONE, DEFAULT_ALIGN);
vb.astCommPool[0].u32BlkCnt = 6;
vb.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
SAMPLE_COMM_SYS_Init(&vb);
```

#### 问题 2: VPSS GetChnFrame 超时 — 缺少专用 VB 池

这是**最关键的根因**。闭库 `mmf_add_vi_channel_v2` 内部调用链：

```
mmf_add_vi_channel_v2 → mmf_set_vi_vflip → ...
  CVI_VPSS_GetGrpAttr(0, &grp_attr)
  CVI_VPSS_SetChnAttr(0, vi_pipe, &chn_attr)
  CVI_VPSS_EnableChn(0, vi_pipe)
  SAMPLE_COMM_VI_Bind_VPSS(vi_pipe, chn, 0)
  create_vpss_pool(...)                  ← 创建 VPSS 专用池
  CVI_VPSS_AttachVbPool(0, vi_pipe, pool_id)  ← 附加池
```

开源 bypass 代码省略了 `create_vpss_pool` + `CVI_VPSS_AttachVbPool`。VPSS 在 `VPSS_INPUT_MEM` 模式下需要独立输出池；没有池时：
- **缩放模式**（1920×1080→640×480）：VPSS 分配中间缩放缓冲区，间接获得输出空间 → 偶然工作
- **直通模式**（1920×1080→1920×1080）：VPSS 尝试零拷贝直通，但无输出池可用 → `GetChnFrame` 永远超时

**修复**: 创建并附加 VPSS 专用池：

```cpp
VPSS_CHN_ATTR_S chn_attr;
if (CVI_SUCCESS == CVI_VPSS_GetChnAttr(0, 0, &chn_attr)) {
    chn_attr.enPixelFormat = (PIXEL_FORMAT_E)_maix_to_mmf_format(_format);
    chn_attr.u32Depth = 2;  // depth=0 默认缓冲区不足
    CVI_VPSS_DisableChn(0, 0);
    CVI_VPSS_SetChnAttr(0, 0, &chn_attr);
    CVI_VPSS_EnableChn(0, 0);
}
// 创建 VPSS 专用池（大小 = 输入帧大小，3 块）
VB_POOL_CONFIG_S vpss_pool_cfg;
memset(&vpss_pool_cfg, 0, sizeof(vpss_pool_cfg));
CVI_U32 vpss_blk = COMMON_GetPicBufferSize(
    vpss_in.u32Width, vpss_in.u32Height,
    SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8,
    COMPRESS_MODE_NONE, DEFAULT_ALIGN);
vpss_pool_cfg.u32BlkSize = vpss_blk;
vpss_pool_cfg.u32BlkCnt = 3;
vpss_pool_cfg.enRemapMode = VB_REMAP_MODE_CACHED;
VB_POOL vpss_pool = CVI_VB_CreatePool(&vpss_pool_cfg);
if (VB_INVALID_POOLID != vpss_pool) {
    CVI_VPSS_AttachVbPool(0, 0, vpss_pool);
}
```

**关键经验**：VPSS 专用池的 `u32BlkSize` 必须用**输入帧大小**（`vpss_in`），而非输出帧大小（`vpss_out`）。如果按输出大小（如 640×480 = 460KB）创建，而 VPSS 内部需要 1920×1080 = 3.1MB 的处理缓冲区，会导致分配失败或死锁。

#### 问题 3: `const char *board_id = sys::device_id().c_str()` use-after-free

```cpp
// 🚫 错误写法：临时 string 在分号后析构，board_id 变成野指针
const char *board_id = sys::device_id().c_str();

// ✅ 正确写法：先存储 string 对象
std::string board_id_str = sys::device_id();
const char *board_id = board_id_str.c_str();
```

`sys::device_id()` 返回 `std::string` 临时对象，`.c_str()` 返回其内部缓冲区的指针。分号后临时对象析构，`board_id` 指向已释放内存。这导致 `Camera(1920, 1080)` 在某些情况下随机"unknown board name!"错误，而 `Camera()` 默认参数可能恰好未覆盖该内存。

此 bug 在原始代码中就已存在（第 918 行），只是闭库路径未触发（闭库不调用 `_mmf_vi_init`，不走 board_id 判断）。

#### 问题 4: VPSS depth=0 导致缓冲不足

`SAMPLE_PLAT_VPSS_INIT` 默认 `u32Depth = 0`，文档注释为"default depth"。实际测试表明 depth=0 时 VPSS 输出缓冲区数量不足以支持连续帧传递，需要通过 `DisableChn` + `SetChnAttr(u32Depth=2)` + `EnableChn` 重启通道。

#### 问题 5: 像素格式编号不一致

`image::Format` 枚举和 `mmf_invert_format_to_maix` 函数使用不同的编号系统：

| 格式 | `image::Format` 值 | `mmf_invert_format_to_maix` 返回值 |
|------|--------------------|-----------------------------------|
| RGB888 | `FMT_RGB888 = 0` | `0`（巧合相同） |
| BGR888 | `FMT_BGR888 = 1` | `1`（巧合相同） |
| NV21/YVU420SP | `FMT_YVU420SP = 8` | `8` |

格式检查代码：
```cpp
// 🚫 原代码：直接转换，可能不匹配
pop_fmt = (image::Format)mmf_invert_format_to_maix(format);

// ✅ 新代码：显式映射
static int _maix_to_mmf_format(image::Format fmt) {
    switch (fmt) {
        case image::FMT_YVU420SP: return PIXEL_FORMAT_NV21;
        case image::FMT_RGB888: return PIXEL_FORMAT_RGB_888;
        case image::FMT_BGR888: return PIXEL_FORMAT_BGR_888;
        default: return PIXEL_FORMAT_NV21;
    }
}
static image::Format _mmf_to_maix_format(int mmf_fmt) {
    switch (mmf_fmt) {
        case PIXEL_FORMAT_NV21: return image::FMT_YVU420SP;
        case PIXEL_FORMAT_RGB_888: return image::FMT_RGB888;
        case PIXEL_FORMAT_BGR_888: return image::FMT_BGR888;
        default: return image::FMT_YVU420SP;
    }
}
```

另外，pybind11 wrapper 生成代码中 Camera 构造函数的默认格式是 `image::FMT_RGB888`（`maixpy_wrapper.cpp:720`），覆盖了 C++ 头文件的默认值。

### 最终管线状态

```
Camera::open() 成功后:

  MIPI RX → ISP → VI PIPE → ISP POST → VI DMA → DDR
                                                    │
                                          (VI_OFFLINE_VPSS_OFFLINE)
                                                    │
                                                    ▼
                                               VPSS (dedicated pool)
                                                    │
                                                    ▼
                                              CVI_VPSS_GetChnFrame
                                                    │
                                                    ▼
                                               cam.read()
```

所有路径验证结果：

| 模式 | 分辨率 | 帧率 | 状态 |
|------|--------|------|------|
| 线性 1080p60 | 1920×1080 | 60 fps (max) | ✅ 连续 5 帧读取成功 |
| 线性 1440p30 | 2560×1440 | 30 fps | ✅ 代码保留（未重新验证） |
| WDR 1440p30 | 2560×1440 | 30 fps | ✅ 代码保留（未重新验证） |

### 涉及文件

| 文件 | 修改 |
|------|------|
| `maix_camera_mmf.cpp` | 新增 VPSS bypass 全部代码：自定义 VB 池、VI 直接初始化、VPSS 创建+池绑定+通道重启、use-after-free 修复、格式映射 |
| `maix_camera.hpp` | 构造函数默认格式从 `FMT_RGB888` 改为 `FMT_YVU420SP` |

### 关键经验总结

1. **VPSS 必须有专用 VB 池**。`CVI_VPSS_AttachVbPool` 不是可选的——闭库 `mmf_add_vi_channel_v2` 内部强制调用。池块大小必须 ≥ VPSS 输入帧大小。
2. **`CVI_VI_EnableChn` 要调两次**：第一次在 `SAMPLE_PLAT_VI_INIT` 内部（`SAMPLE_COMM_VI_StartViChn`），第二次在 init 后显式调用（确保状态）。
3. **`VPSS_CHN_ATTR_S.u32Depth` 不能为 0**。设为 2 以上确保足够输出缓冲区。
4. **`string::c_str()` 的陷阱**：`sys::device_id().c_str()` 是 C++ 经典 use-after-free。必须先赋值给 `std::string` 变量。
5. **格式编号不要靠巧合**。`mmf_invert_format_to_maix` 和 `image::Format` 用各自的编号系统，必须显式映射。
