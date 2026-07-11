# OS04A10 1080p60 + WDR 模式开发记录

> 最后更新: 2026-07-11 | 设备: MaixCAM Pro (SG2002/CV1813H) + OS04A10

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

## 二、WDR 模式（实验性，VI 管道已验证通过）

### 目标
为 OS04A10 添加 2560×1440 WDR (Wide Dynamic Range) 模式，使用 DCG+VS 双曝光 staggered HDR。

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

### 当前状态（2026-07-11）

```
传感器初始化 → ✅ MIPI PHY 时钟 HS → ✅ CSI HDR_EN=1 → ✅ VI 双通道接收 → ✅ ISP FSWDR 合并 → ✅
  → VI→VPSS 绑定 → ❌ (CVI_VPSS_GetChnFrame 超时)
  → 应用层帧读取 → ❌ (cam.read() 超时)
```

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
# WDR 模式
MAIX_WDR_MODE=2to1_line python3 -c "
from maix import camera
cam = camera.Camera(2560, 1440)
# Camera() 成功返回，但 cam.read() 超时（VPSS 绑定待修复）
"

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

| # | 问题 | 优先级 | 可能方案 |
|---|------|--------|---------|
| 1 | VPSS 绑定：FSWDR 合并帧未路由到 VPSS | 高 | 调整 `mmf_add_vi_channel_v2` 的通道索引或 VPSS 组配置 |
| 2 | ISP bin sensor ID 不匹配：5440579→5440577 | 中 | 二进制 patch bin offset 815 |
| 3 | ISP bin MD5 不匹配：JSON 回退（慢但功能相同） | 低 | 二进制 patch bin offset 660 或重建 libcvi_bin_isp |
| 4 | 帧率 7 FPS（应 30 FPS） | 中 | AE 将 VTS 提高到 1948（25fps 目标），可能需 I2C VTS 覆写 |
| 5 | 缺少 WDR 场景调优（BLC/AWB/etc. 参数来自 OV04A10 bin） | 低 | 需要 OS04A10 专用 WDR 校准 bin |

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
| VI teardown | `maix_camera_mmf.cpp:808` |
| CSI 覆写 | `maix_camera_mmf.cpp:825` |
| ISP bin | `/mnt/cfg/param/cvi_wdr_bin.os04a10` |
| 编译部署 | `maixpy_wdr_build.md` |

### 修改提交

```
e9e44ca7 wdr: skip VPSS first-frame wait for WDR mode, add VI pipe fallback
1e9e125a add os04a10 1440p30 WDR mode support (experimental)
```

---

## 三、关键发现

### 关于"CSI 无法锁定帧时序"的再评估

之前分析文档（2026-06）声称 SG2002 CSI bit19=1 (VC帧检测) 且 bit20=0 (帧锁定失败)。
TRM 和驱动源码分析证实 **SG2002 CSI 没有 bit19/bit20 状态位**。这些位来自其他平台。

经过修复后的 WDR 模式测试证明：
- CSI HDR_EN=1 正常生效
- VI 两个 ISP 通道都收到帧（VISofCh0Cnt=8, VISofCh1Cnt=8）
- ISP FSWDR 成功合并帧（VIPostCnt=7）

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
