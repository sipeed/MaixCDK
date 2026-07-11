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
