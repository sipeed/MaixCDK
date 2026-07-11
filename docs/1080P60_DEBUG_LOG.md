# OS04A10 1080p60 模式完整工作记录

## 目标

为 MaixCAM Pro 上的 OS04A10 传感器添加 1920×1080 @ 60fps 模式，通过完整的 VI→ISP→VPSS 管线。

## 背景

OS04A10（OmniVision 4MP）原始支持两个线性模式：
- 1440p30（2560×1440 @ 30fps，从 2688×1520 裁剪）
- 720p90（1280×720 @ 90fps，从 2688×1520 裁剪，实际运行 80fps）

目标增加 1080p60（1920×1080 @ 60fps，RAW12，4-lane MIPI）。

## 环境

- 硬件: MaixCAM Pro（无显示，无摄像头屏幕）
- SoC: SG2002（RISC-V C906, big core Linux + little core RTOS）
- 框架: MaixCDK + MaixPy
- 传感器: OS04A10 on I2C bus 4, addr 0x36
- 工具链: riscv64-unknown-linux-musl-gcc
- 系统: Linux 5.10.4

## 管线架构

```
Sensor (OS04A10) → MIPI RX (VI DEV) → ISP → VI PIPE → VPSS → 应用
     ↑                    ↑            ↑       ↑         ↑
  1920×1080@60         尺寸一致     正确配置   缓冲正确   缩放输出
```

每个管线阶段必须配置相同分辨率。仅覆盖传感器层会导致 `vi get frame timeout`。

## 修改的文件

### MaixCDK 传感器驱动

| 文件 | 修改内容 |
|------|----------|
| `components/3rd_party/sophgo-middleware/v2/component/isp/sensor/sg200x/ov_os04a10/os04a10_cmos_ex.h` | 添加 `OS04A10_MODE_1080P60_12BIT` 枚举 |
| `components/.../ov_os04a10/os04a10_cmos_param.h` | 添加 1080p60 模式参数表（HTS=1484, VTS=1216, 1920×1080） |
| `components/.../ov_os04a10/os04a10_sensor_ctl.c` | 添加 `os04a10_linear_1080p60_12BIT_init()` 寄存器序列（302+ 个 I2C 写入） |
| `components/.../ov_os04a10/os04a10_cmos.c` | 修改 `cmos_set_image_mode()` 添加 1080p60 分支 + /tmp/force_1080p60 调试开关 |

### MaixCDK 中间件层

| 文件 | 修改内容 |
|------|----------|
| `components/.../sample/common/sample_comm.h` | 添加 `OV_OS04A10_MIPI_4M_1080P60_12BIT` 枚举 |
| `components/.../sample/common/sample_common_sensor.c` | 添加 sensor→size/fps/obj 映射 + 修复 `snsr_type_name[]` 数组 |
| `components/vision/port/maixcam/maix_camera_mmf.cpp` | 添加 1080p60 模式选择逻辑 + FPS 钳位更新 |

### 修复的 Bug（已有）

| 问题 | 文件 | 行号 | 修复 |
|------|------|------|------|
| `snsr_type_name[]` 缺少 `GCORE_OV2685_MIPI_1600x1200_30FPS_10BIT` | sample_common_sensor.c | ~224 | 添加缺失字符串 |
| `OV_OS04A10_MIPI_4M_1080P60_12BIT` 后面缺少逗号 | sample_common_sensor.c | 226 | 添加 `,` |
| 缺少 NULL 防护 | sample_common_sensor.c | 1891 | `snsr_type_name[i] != NULL` |

### 构建产物

| 组件 | 路径 | 大小 | 部署路径 |
|------|------|------|----------|
| `libsophgo-middleware.so` | `MaixPy/build/sophgo-middleware/` | 350KB | `/usr/lib/python3.11/site-packages/maix/dl_lib/` |
| `_maix.so` | `MaixPy/build/maix/` | 13.4MB | `/usr/lib/python3.11/site-packages/maix/` |

### Git

```
MaixCDK: d277c398 "add OS04A10 1080p60 sensor mode + maixcam camera selection"
```

## 构建步骤

```bash
# 1. 修改传感器驱动代码
# 2. 编译 sophgo-middleware
cd MaixPy && mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCONFIG_MAIXCAM_PRO=ON
make sophgo-middleware -j4

# 3. 编译 _maix.so（需要 freetype 交叉编译）
# freetype 编译：--with-pic 用于 RISC-V 共享库
make maix -j4

# 4. 部署
scp build/sophgo-middleware/libsophgo-middleware.so root@board:/usr/lib/python3.11/site-packages/maix/dl_lib/
scp build/maix/libmaix.so root@board:/usr/lib/python3.11/site-packages/maix/_maix.so
```

## 配置

### sensor_cfg.ini 位置: `/mnt/data/sensor_cfg.ini`

1440p30 配置（工作状态）:
```ini
[source]
dev_num = 1
[sensor]
name = OV_OS04A10_MIPI_4M_1440P_30FPS_12BIT
bus_id = 4
sns_i2c_addr = 29
mipi_dev = 0
lane_id = 4, 3, 2, -1, -1
pn_swap = 0, 0, 0, 0, 0
mclk_en = 1
mclk = 1
```

## 测试结果

### Camera(640, 360, fps=30) → 1440p30 模式
```
stSnsrMode ... 2560x1440 25fps
Init OK
OPEN OK
FRAME 0: 640x360
DONE
```
✅ 正常工作

### Camera(1920, 1080, fps=30) → 1440p30 + VPSS 缩放
```
stSnsrMode ... 2560x1440 25fps
Init OK
OPEN OK
F30 0: 1920x1080
DONE
```
✅ VPSS 缩放工作正常

### Camera(1920, 1080, fps=60) → 1080p60 模式（通过 _maix.so 重写）
```
stSnsrMode ... 1920x1080 60fps
cmos_set_image_mode fps:60
Init OK (1080P 60fps)
[open]-903: vi get frame timeout: 0xc006800e !
```
❌ 帧超时

### Camera(640, 360) → 1080p60 ini（单次初始化）
```
sensor = OV_OS04A10_MIPI_4M_1080P60_12BIT
found at index 136
stSnsrMode ... 1920x1080 60fps
Init OK
[open]-903: vi get frame timeout: 0xc006800e !
```
❌ 同样是帧超时（非双初始化问题）

## 传感器寄存器验证

通过 `i2ctransfer` 回读传感器寄存器：
```
VTS = 0x04C0 = 1216     ✓ (正确: 1484×1216×60 ≈ 108MHz)
HTS = 0x05CC = 1484     ✓
输出宽度 = 0x0780 = 1920 ✓
输出高度 = 0x0438 = 1080 ✓
流启停 = 0x01            ✓ (已开始流)
```

## SIGSEGV 根因分析

### 现象
ini 中设置 `name = OV_OS04A10_MIPI_4M_1080P60_12BIT` 时，`parse_sensor_name()` 循环到 i=175 时崩溃。

### 原因链
1. 枚举 `SAMPLE_SNS_TYPE_E` 有 180 个条目（索引 0-179），`SAMPLE_SNS_TYPE_BUTT = 179`。
2. 数组 `snsr_type_name[]` 应初始化 179 个条目。
3. 但我们的 `"OV_OS04A10_MIPI_4M_1080P60_12BIT"` **缺少尾部逗号**，导致 C 编译器将它与下一行 WDR 字符串拼接成一个。
4. 结果：整个 WDR 段偏移 1 位，最后 2 个 WDR 条目为 NULL。
5. 循环到 NULL 时 `strcmp` 解引用 NULL → SIGSEGV。

### 额外问题
枚举中有 `GCORE_OV2685_MIPI_1600x1200_30FPS_10BIT` 但在数组中缺失，导致后续条目偏移 +1。

### 修复
1. 在 sample_common_sensor.c 数组中添加缺失的 `"GCORE_OV2685_MIPI_1600x1200_30FPS_10BIT"`。
2. 在行末添加逗号。
3. 在循环中添加 `snsr_type_name[i] != NULL` 防护。

## 帧超时分析

### 排除的原因
- ❌ ISP bin 不匹配：移除 bin 后同样超时
- ❌ MIPI 数据率不匹配：PLL 配置与 1440p30 相同（108 MHz 像素时钟）
- ❌ 双初始化问题：单次 ini 直接指定 1080p60 同样超时
- ❌ 传感器配置错误：I2C 回读确认寄存器值正确
- ❌ 管线大小不匹配：单次初始化时 VI DEV/PIPE/ISP/VPSS 均配置为 1920×1080

### 可能的原因
- ❓ ISP 吞吐量上限：1080p60 = 124M 像素/秒 vs 1440p30 = 110M 像素/秒（+12%）
- ❓ VI pipe DMA 无法处理 1920×1080@60 的数据速率
- ❓ MIPI D-PHY hs_settle 时间在 1080p60 模式需要调整
- ❓ 内核 VI 驱动有分辨率限制（line buffer 仅限 2560 宽度）

## 关键架构发现

### 初始化流程
```
mmf_init0(0x01, false)          ← libmaixcam_lib.so（预编译，无源码）
  → _mmf_init()                 ← sophgo_middleware.c（未编译入 libsophgo-middleware.so）
  → SAMPLE_PLAT_VI_INIT()       ← sample_common_platform.c（在 libsophgo-middleware.so 中）
    → SAMPLE_COMM_VI_StartDev()
    → CVI_VI_CreatePipe()
    → SAMPLE_COMM_VI_CreateIsp()
      → CVI_ISP_SetPubAttr()
      → CVI_ISP_Init()
      → SAMPLE_COMM_BIN_ReadParaFrombin()
      → SAMPLE_COMM_ISP_Run()
        → cmos_set_image_mode()  ← 传感器驱动回调
        → cmos_init()            ← 传感器驱动回调
    → SAMPLE_COMM_VI_StartViChn()
```

### 库依赖链
```
_maix.so (Python C 扩展)
  → libmaixcam_lib.so（预编译，包含 mmf_init0 / mmf_vi_init0）
    → libsophgo-middleware.so（包含 SAMPLE_COMM_* / 传感器驱动）
      → CVI SDK .so（libvi.so, libisp.so, libvpss.so 等）
```

### 双初始化模式
`Camera::open()` 内部调用 `_mmf_vi_init()`，其流程：
1. `SAMPLE_COMM_VI_ParseIni(&stIniCfg)` — 解析 ini 到本地配置
2. 覆盖 `stIniCfg.enSnsType[0] = 1080p60` — 用我们的模式
3. `mmf_init_v2(false)` — 用 ini 配置做第一次完整初始化（VI+ISP+传感器）
4. `mmf_vi_init_v2()` — 用我们的配置做第二次 VI 初始化

步骤 3 和 4 之间存在传感器类型不一致的可能性。但实测单次 ini 直接使用 1080p60 类型也失败，排除了双初始化导致的问题。

### snsr_type_name 查找机制
`parse_sensor_name()` 在 `sample_common_sensor.c` 中：
1. 接受来自 ini 文件的字符串值
2. 遍历 `snsr_type_name[]` 数组（索引与 `SAMPLE_SNS_TYPE_E` 枚举对应）
3. `strcmp` 匹配成功后设置 `cfg->enSnsType[index] = i`
4. 同时 `setenv("SENSORNAME0", value)` 供 `_mmf_vi_init()` 中的 `getenv` 读取

## 第二阶段研究（2026-07-11）

### 子智能体研究成果

#### SG2002 TRM 关键发现
- VI 单传感器最大支持 5M (2688x1944) @30fps 线性 / @60fps HDR
- MIPI RX 1.5Gbps/lane，无分辨率相关硬件限制
- VDP 硬限 1920×1080（但显示非必需）
- VPSS SC_V1 最大 2880 宽，SC_D/SC_V2 最大 1920 宽
- CV180x（相当于 SG2002）：最大 4M (2560×1440) @30fps
- VI_PIPE1_MAX_WIDTH = 2880

#### ISP 参考手册关键发现
- ISP_PUB_ATTR_S 无明确分辨率限制
- bin 文件中 LSC/DPC 表是分辨率相关的
- AE 网格: 17×15 区，AWB 网格: 最大 32×64 区
- ISP 管线 30 级：BLC→RLSC→FPN→DPC→Crosstalk→WBG→DIS→BNR→Demosaic→...

#### MIPI 指南和传感器调试关键发现
- hs_settle 公式: `MAC_Freq * pixel_width = lane_num * MIPI_Freq * 2`
- hs_settle 调试: `/proc/mipi-rx` 检查 EccErr/CrcErr/WcErr
- 时钟通道诊断: CK_HS=1 表示时钟 OK
- 帧超时诊断流: I2C → Sensor ID → MIPI PHY → vi_dbg → Raw dump
- OS04A10 I2C 地址: 0x36，寄存器 2 字节地址 + 1 字节数据

#### Media Processing API 关键发现
- **0xC006800E = CVI_ERR_VPSS_BUF_EMPTY** — VPSS 缓冲区为空
- VI_PIPE_ATTR_S.u32MaxW/H 是静态属性，创建后不可更改
- CV180x: 1 VI DEV, 1 PHY_PIPE, 1 PHY_CHN
- VPSS 支持 32x 缩放
- VI 不支持帧率控制
- 对齐要求: 64 字节

### MIPI RX 诊断结果

通过 `/proc/cvitek/vi_dbg` 和 `/proc/mipi-rx` 获取的关键诊断数据:

```
VISofCh0Cnt: 229          ← 帧起始递增! MIPI RX 收到 SoF
VIPreFECh0Cnt: 0          ← ISP PreFE 计数=0! 数据未进入 ISP
VIFPS: 0                  ← 帧率为 0
VICsiCh0HeightLSCnt: 32   ← 帧高度比预期少! CSI 高度不匹配
VIWdma0ErrStatus: 0x3000000  ← DMA 引擎全部带错误空闲
decode: unknown           ← MIPI RX 无法识别数据类型
CK_HS: 1, 数据通道: hs_idle ← 时钟 OK 但数据无流量
```

结论: **MIPI RX 收到帧起始短包 (SoF) 但无法接收长包 (像素数据)**。CSI 发现每帧行数少 ~32 行。DMA 无数据传输。

### USE_USER_SEN_DRIVER 发现

`cvi_comm_sns.h:27` 定义 `#define USE_USER_SEN_DRIVER 1`，导致 `SAMPLE_PLAT_VI_INIT` 调用:
1. `SAMPLE_COMM_VI_StartSensor()` — patche sensor RX 属性
2. `SAMPLE_COMM_VI_StartMIPI()` — 配置 MIPI RX 硬件

这改变了初始化路径，导致 ini 中的 `lane_id` 和 `pn_swap` 被写入 MIPI RX 硬件。

ini 默认 `lane_id = 4, 3, 2, -1, -1`（仅 2 通道），但 OS04A10 需要 4 通道 `lane_id = 2, 1, 3, 0, 4`。即使修复后，`HeightLSCnt` 错误仍存在。

### 根本原因评估

最可能原因: **MIPI D-PHY hs_settle 值对 1080p60 不优化**。尽管像素时钟与 1440p30 相同，但 60fps 导致 HS/LP 转换频率翻倍，需要不同的 D-PHY 建立时间。hs_settle 寄存器 (0x0300b048) 在用户空间不可访问（返回 DEADBEEF）。

### 更新后的下一步建议

1. **hs_settle 测试**：修改 `os04a10_cmos_param.h` 中的 `.dphy.hs_settle` 值（当前=8），尝试 6-16 范围，重建 `libsophgo-middleware.so`，测试 1080p60 帧率
2. **内核 MIPI 驱动分析**：检查 `drivers/media/platform/cvi_vip/` 下的 MIPI RX 内核驱动源码，确认 lane_id/pn_swap/hs_settle 的实际写入过程
3. **ISP 帧率降级测试**：将 ISP f32FrameRate 设为 30（传感器仍 60fps），确认是否是 ISP 吞吐量瓶颈
4. **直接 ioctl 测试**：编写最小 C 程序，跳过中间件直接调用 CVI SDK API，隔离问题层
5. **CviPQtool 校准**：为 1080p60 生成独立 ISP bin 文件

## 参考文档

- `os04a10_sensor_ctl.c` — 传感器寄存器初始化序列
- `os04a10_cmos.c` — 传感器驱动回调（cmos_set_image_mode, sensor_rx_attr 等）
- `sample_common_vi.c` — VI 初始化函数（StartDev, StartMIPI, CreateIsp 等）
- `sample_common_platform.c` — `SAMPLE_PLAT_VI_INIT` 完整流程
- `sample_common_sensor.c` — snsr_type_name 数组、GetSize/GetDevAttr/GetIspAttrBySns
- `sophgo_middleware.c` — mmf_init / mmf_add_vi_channel（v1 API，部分未使用）
- `sophgo_middleware.hpp` — mmf_init0/mmf_vi_init0 的 dispatch 包装
- `maix_camera_mmf.cpp` — Camera::open / _mmf_vi_init / 模式选择逻辑

---

## 第三阶段研究（2026-07-11）— 内核驱动源码 + 硬件寄存器转储

### MIPI RX / CIF 内核驱动源码分析

#### 驱动文件位置

所有的 MIPI RX（CIF）和 VI（ISP）内核驱动源码位于：
```
osdrv/interdrv/v2/cif/chip/mars/     ← soph_mipi_rx.ko
osdrv/interdrv/v2/vi/chip/mars/      ← soph_vi.ko
```

关键文件：
| 文件 | 作用 |
|------|------|
| `cif/chip/mars/cif.c` | CIF 平台驱动，/proc/mipi-rx 读写处理，ioctl 处理 |
| `cif/chip/mars/drv/cif_drv.c` | CIF 硬件驱动：CSI/SubLVDS/HiSPi 配置，HS settle，streaming |
| `cif/chip/mars/drv/inc/reg_fields_csi_mac.h` | CSI MAC 寄存器位域定义（1311 行） |
| `cif/chip/mars/drv/inc/reg_fields_csi_wrap.h` | PHY WRAP 寄存器位域定义（1686 行） |
| `include/chip/mars/uapi/linux/cif_uapi.h` | combo_dev_attr_s, raw_data_type_e, rx_mac_clk_e 等 UAPI 定义 |
| `vi/chip/mars/vi.c` | VI 驱动：streaming start/stop, ISP pipeline, DMA, proc fs |
| `vi/chip/mars/proc/vi_dbg_proc.c` | /proc/cvitek/vi_dbg 处理器 |
| `vi/chip/mars/vip/vi_drv.c` | ISP VIP 硬件驱动：FE/BE/DMA/BLC/WBG 等模块配置 |
| `vi/chip/mars/vip/vi_fe_ip_ctrl.c` | CSI 桥接器（CSIBDG）配置（尺寸/时序/裁剪） |

#### 寄存器地址映射（已验证通过 devmem 转储）

| 模块 | 基地址 | 说明 |
|------|--------|------|
| ISP TOP | 0x0A000000 | ISP 子系统顶层 |
| ISP CSIBDG0 | 0x0A000800 | CSI 桥接器通道0 |
| CIF MAC0 TOP | 0x0A0C2000 | 传感器 MAC 控制器 |
| CIF MAC0 CSI | 0x0A0C2400 | CSI 控制器顶层 |
| CIF MAC0 VI | 0x0A0C2600 | MAC 到 VI 接口 |
| PHY WRAP TOP | 0x0A0D0000 | D-PHY 包装器顶层 |
| PHY WRAP 4L | 0x0A0D0300 | 4通道 D-PHY 寄存器 |
| PHY WRAP 2L | 0x0A0D0600 | 2通道 D-PHY 寄存器 |

#### 关键 CSI 寄存器解释

**REG_CSI_CTRL_TOP_40** (0x0A0C2440) — 错误状态 + 解码格式（只读）：
```
bits 0:  CSI_ECC_NO_ERROR        — ECC 无错误
bits 1:  CSI_ECC_CORRECTED_ERROR — ECC 已纠正
bits 2:  CSI_ECC_ERROR           — ECC 不可纠正错误
bits 4:  CSI_CRC_ERROR           — CRC 错误
bits 5:  CSI_WC_ERROR            — Word Count 错误
bits 8:  CSI_FIFO_FULL            — FIFO 满
bits 21:16 CSI_DECODE_FORMAT     — 实际解码的数据类型：
  bit[0]=YUV422_8bit, [1]=YUV422_10bit, [2]=RAW8, [3]=RAW10, [4]=RAW12, [5]=RAW16
```

**REG_CSI_CTRL_TOP_04** (0x0A0C2404) — HDR/中断控制：
```
bits 7:0  CSI_INTR_MASK          — 中断屏蔽
bits 15:8 CSI_INTR_CLR           — 中断清除（写1清除）
bit 16    CSI_HDR_EN             — HDR 模式使能
bit 17    CSI_HDR_MODE           — HDR 模式选择：0=VC模式, 1=ID(DT)模式
bit 18    CSI_ID_RM_ELSE         — 移除 ELSE ID
bit 19    CSI_ID_RM_OB           — 移除 OB ID
```

**CSI 桥接器中断状态** (0x0A0008E0) — `VICsiIntStatus0` 的来源：
```
bit 0: CH0_FRAME_DROP_INT    — 帧被丢弃
bit 1: CH0_VS_INT            — VSync 中断
bit 4: CH0_FRAME_WIDTH_GT_INT — 帧宽度 > 期望
bit 5: CH0_FRAME_WIDTH_LS_INT — 帧宽度 < 期望
bit 6: CH0_FRAME_HEIGHT_GT_INT — 帧高度 > 期望
bit 7: CH0_FRAME_HEIGHT_LS_INT — 帧高度 < 期望
```

### `decode: unknown` 的硬件级解释

`cif_get_csi_decode_fmt()` 函数读取 `CSI_DECODE_FORMAT`（0x0A0C2440 的 bits 21:16）：
```c
uint32_t value = CIF_RD_BITS(mac_csi, REG_CSI_CTRL_TOP_T, REG_40, CSI_DECODE_FORMAT);
for (i = 0; i < DEC_FMT_NUM; i++) {
    if (value & (1 << i))
        return i;  // 返回 DEC_FMT_RAW12 (=4), RAW10 (=3) 等
}
return i;  // 返回 DEC_FMT_NUM (=5) → 显示 "unknown"
```

当 `value = 0` 时，所有位为 0，返回 "unknown"。这意味着 CSI 控制器从传入的 CSI-2 长包**未能识别出任何已知数据格式**。

### `VICsiIntStatus0 = 0x3` 的硬件级解释

```c
union REG_ISP_CSI_BDG_DVP_INTERRUPT_STATUS_0 {
    bit 0: CH0_FRAME_DROP_INT = 1  ← 帧被丢弃！
    bit 1: CH0_VS_INT = 1          ← 正常 VSync
};
```

CSI 桥接器在接收帧时检测到尺寸/格式不匹配，**直接丢弃整帧**。

### `VICsiCh0HeightLSCnt = 32` 含义

这是 `bdg_h_ls_cnt[ISP_FE_CH0]`，一个**累加计数器**，不是存储收到的实际高度。每次帧高度 < 期望值时 +1。值=32 表示 32 帧都被检测为高度不足。

### 驱动代码配置问题

`_cif_hdr_csi_enable()` 在 WDR_MODE_NONE（linear）模式下：

```c
if (param->hdr_mode == CSI_HDR_MODE_VC) {       // 1
} else if (param->hdr_mode == CSI_HDR_MODE_DT) { // 2
    // 只有此处写入 decode_type (0x2C=RAW12)
    CIF_WR_BITS(..., CSI_HDR_DT_FORMAT, param->decode_type);
} else if (param->hdr_mode == CSI_HDR_MODE_DOL) { // 3
} else {  // hdr_mode=0 (NONE) 走到这里！
    CIF_WR_BITS(..., CSI_HDR_MODE, 1);  // 设置 HDR ID 模式
}
CIF_WR_BITS(..., CSI_HDR_EN, 0);  // HDR 禁用
```

但是实际硬件转储显示 `CSI_HDR_CTRL = 0x000C0000`：
- HDR_MODE = **0**（不是 1！）
- HDR_EN = 0 ✓
- ID_RM_ELSE = 1, ID_RM_OB = 1

这意味着驱动的 `else` 分支写入 HDR_MODE=1 并没有生效，或者被后续覆盖。1440p30 实测 HDR_MODE=0 完全正常工作。

### 1440p30 工作模式的完整寄存器快照

通过 devmem 实时转储（Camera 运行时）：

```
=== CSI CTRL TOP (0x0A0C2400) ===
0x0A0C2400 (CSI_LANE_MODE):     0x00000003  → 4-lane (bits=011), ECC on, VC check off
0x0A0C2404 (CSI_HDR_CTRL):      0x000C0000  → HDR_EN=0, HDR_MODE=0, RM_ELSE=1, RM_OB=1
0x0A0C2440 (CSI_ERR_DECODE):    0x00100001  → ECC_NO_ERROR, DECODE=RAW12(bits 21:16=0x10=bit4)
0x0A0C2470 (CSI_VS_GEN):        0x00000F00  → VS gen mode
0x0A0C2474 (CSI_HDR_DT):        0x00000000  → HDR DT 未配置

=== SENSOR MAC TOP (0x0A0C2000) ===
0x0A0C2000 (SENSOR_MAC_TOP):    0x00006E71  → bits: CSI mode(1:0=01), VS_INV(2=1), HS_INV(4=1),
                                               CSI_CTRL_EN(6=1), SW_UP(10=1)...

=== PHY WRAP 4L (0x0A0D0300) ===
0x0A0D0300 (PHY_4L_00):         0x00000000  → SENSOR_MODE=0 (CSI), MIPIRX_PD_IBIAS=0, MIPIRX_PD_RXLP=0
0x0A0D0304 (PHY_4L_04):         0x00000413  → D0_SEL=3, D1_SEL=1, D2_SEL=4
0x0A0D0308 (PHY_4L_08):         0x00000F12  → D3_SEL=2, CK_SEL=1, CK_PNSWAP=1
0x0A0D030C (PHY_4L_0C):         0x00FF000F  → DESKEW_LANE_EN
0x0A0D0310 (PHY_4L_10):         0x00000208  → AUTO_IGNORE=0, AUTO_SYNC=0
0x0A0D03A4 (PHY_4L_A4):         0x60202001  → per-lane DPHY settings, T_HS_SETTLE and other timing

=== ISP CSIBDG0 (0x0A000800) ===
0x0A000800 (CSIBDG0_00):        0x11300001  → ch0/1/2/3 DMA write enabled, abort/reset status
0x0A000810 (CH0_SIZE):          0x05EF0A7F  → width=2687(2688-1), height=1519(1520-1) ← 原生尺寸
0x0A000820 (CROP_EN):           0x00000001  → crop enabled
0x0A000824 (HCROP):             0x0A3F0040  → start_x=64, end_x=2623(=64+2560-1) ← 有效宽度2560
0x0A000828 (VCROP):             0x05C70028  → start_y=40, end_y=1479(=40+1440-1) ← 有效高度1440
0x0A000880 (MDE_V):             0x00000000  → motion detect V size (not configured)
0x0A000884 (MDE_H):             0x00000000  → motion detect H size (not configured)
0x0A000888 (FDE_V):             0x00000000  → fake disparity V size (not configured)
0x0A00088C (FDE_H):             0x00000000  → fake disparity H size (not configured)
0x0A000898 (TGEN):              0x00000CCD  → timing generator VTT=3277, HTT=0
0x0A0008E0 (INT0):              0x00000000  ← 无中断！无帧丢弃
0x0A0008E4 (INT1):              0x00000000  ← 无溢出
0x0A0008E8 (DBG):               0x00000001  ← ring buffer idle=1 (正常)
0x0A0008A0 (CH0_DBG0):          0x37C4B800  ← CSI 桥运行状态
0x0A0008A4 (CH0_DBG1):          0x000005EF  ← 收到行数=1519 (=1520-1) ✓
0x0A0008A8 (CH0_DBG2):          0x00E900E9  ← 每行边界信息
0x0A0008AC (CH0_DBG3):          0x00000004  ← 通道状态
```

### 720p90 模式的实质发现

查看 `cmos_set_image_mode()` 代码（os04a10_cmos.c:1167-1219）：

```c
if (pstSensorImageMode->f32Fps <= 30) {
    // 选择 1440p30 或 WDR
} else {
    // fps > 30: else 分支为空！！！
}
```

fps=90 > 30，进入空 else，`u8SensorImageMode` 保持默认值 0（= OS04A10_MODE_1440P30_12BIT）。

**720p90 不改变传感器配置！** 传感器仍以 2560×1440 输出完整帧，ISP/VPSS 后级做缩放。
因此 720p90 的 MIPI RX / CSIBDG 寄存器与 1440p30 完全一致。

### `1024 x 768` 感兴区域实验（中间结论）

在调试日志中记录但未证实的假设：将 crop 设置为传感器原生矩形内任意的 1920-width 子区域，传感器内部 ISP 输出格式可能不同。

### 所有 1080p 传感器的 MIPI 配置对比

| 传感器 | mac_clk | raw_type | 通道 | MCLK | hs_settle | VTS | HTS |
|--------|---------|----------|------|------|-----------|-----|-----|
| **GC2093 1080p60** | RX_MAC_CLK_200M | RAW10 | 3-lane | 27MHz | (默认) | 1250 | 2200 |
| **IMX327 1080p60** | RX_MAC_CLK_200M | RAW12 | 4-lane | 37.125MHz | (默认) | 1125 | 4400 |
| **OS04A10 1440p30** | RX_MAC_CLK_400M | RAW12 | 4-lane | 25MHz | 8 | 2432 | 1484 |
| **OS04A10 1080p60** | RX_MAC_CLK_400M | RAW12 | 4-lane | 25MHz | 8 | 1216 | 1484 |

OS04A10 是唯一使用 `RX_MAC_CLK_400M` 的传感器。当测试改为 `RX_MAC_CLK_200M` 时，SoF 完全消失（MIPI 链路断开）。

### 当前根因评估

**最可能原因：OS04A10 在 1920×1080 输出模式下，CSI-2 长包的数据类型标识符（DT）与 2560×1440 模式不同，导致 CSI 控制器 DECODE_FORMAT 全零。**

具体来说，OS04A10 内部 ISP 在输出分辨率变化时可能改变了以下之一：
1. MIPI 输出数据格式（RAW12 → 其他 DT）
2. Virtual Channel 标签（0 → 其他）
3. 包结构（non-continuous clock 表现不同）

**直接验证实验（未执行）：**
在 1440p30 运行时，通过 i2ctransfer 在线修改传感器寄存器到 1080p60 尺寸，立即观察 CSI_DECODE_FORMAT 变化：
```bash
i2ctransfer -y -f 4 w3@0x36 0x38 0x08 0x07  # OUT_W_H = 0x07 (1920>>8)
i2ctransfer -y -f 4 w3@0x36 0x38 0x09 0x80  # OUT_W_L = 0x80 (1920&0xFF)
i2ctransfer -y -f 4 w3@0x36 0x38 0x0a 0x04  # OUT_H_H = 0x04 (1080>>8)
i2ctransfer -y -f 4 w3@0x36 0x38 0x0b 0x38  # OUT_H_L = 0x38 (1080&0xFF)
i2ctransfer -y -f 4 w3@0x36 0x38 0x0e 0x04  # VTS_H = 0x04 (1216>>8)
i2ctransfer -y -f 4 w3@0x36 0x38 0x0f 0xc0  # VTS_L = 0xC0 (1216&0xFF)
cat /proc/mipi-rx | grep decode
cat /proc/cvitek/vi_dbg | grep -E "Height|Width|VIFPS|decode"
```

如果在线切换后 decode 变为 raw12 → 问题在传感器 init 序列中其他寄存器
如果 decode 仍为 unknown → 问题在传感器固件/硬件对 1080p 输出的限制

---

## 第四阶段研究（2026-07-11）— 完整地址空间扫描 + 突破性测试

### MIPI RX 完整寄存器地址空间扫描

使用 devmem 对整个 MIPI RX 地址空间进行了全面扫描，覆盖范围：

| 地址范围 | 模块 | 扫描结果 |
|----------|------|----------|
| 0x0A0C2000-0x0A0C21FF | MAC0_TOP (sensor misc) | 有非零寄存器 ✅ |
| 0x0A0C2200-0x0A0C23FF | MAC0_SLVDS (Sub-LVDS/HiSPi) | 有非零寄存器 ✅ |
| 0x0A0C2400-0x0A0C25FF | MAC0_CSI (CSI控制器) | 已详细分析 ✅ |
| 0x0A0C2600-0x0A0C27FF | MAC0_VI (MAC→VI) | **全部为零** |
| 0x0A0C4000-0x0A0C41FF | MAC1_TOP | 有非零（未使用链路） |
| 0x0A0C4200-0x0A0C43FF | MAC1_SLVDS | 有非零（未使用链路） |
| 0x0A0C4400-0x0A0C45FF | MAC1_CSI | 有非零（未使用链路） |
| 0x0A0C6000-0x0A0C61FF | MAC2 (BT only) | 有非零 |
| 0x0A0D0000-0x0A0D02FF | PHY_WRAP_TOP | 有非零 ✅ |
| 0x0A0D0300-0x0A0D05FF | PHY_WRAP_4L | 有非零 ✅ |
| 0x0A0D0600-0x0A0D08FF | PHY_WRAP_2L | 有非零 |
| 0x0A0D1000-0x0A0D107F | MIPI_TX_PHY | 有非零（显示输出） |

**没有发现任何隐藏或未归档的寄存器。** 所有非零寄存器都在已知模块内。

### MAC0_CSI (工作链路) vs MAC1_CSI (未使用) 对比

| 寄存器 | MAC0 (Link 0 活跃) | MAC1 (Link 1 未用) | 含义 |
|--------|-------------------|-------------------|------|
| 0x..00 | 0x00000003 | 0x00000000 | 仅L0配置了4-lane |
| 0x..04 | 0x000C0000 | 0x000C0000 | HDR配置相同 |
| 0x..40 | 0x00100001 | 0x00000000 | L0解码出RAW12 |
| 0x..70 | 0x00000F00 | 0x00000F02 | 仅bit1(VS_GEN_MODE)不同 |
| 0x..08~0x..24 | 多种值 | **完全相同** | ECC/VC/短包配置完全一致 |

**CSI 配置在 MAC0 和 MAC1 的 0x08-0x24 区段完全一致。** 说明这些寄存器不是导致 decode 差异的原因。

### CSI Lane 强制复位实验

在 1080p60 传感器配置（在线切换）下，强制写 0x0A0C2400：
- 写 0x00000000（禁用lane）→ CSI_DECODE 变为 `0x00100004`（ECC错误触发）
- 写 0x00000003（恢复4-lane）→ CSI_DECODE 恢复为 `0x00100001`（RAW12+无错误）
- **CSI 控制器在任何时候都能正确重新检测 RAW12**
- VIFPS=0, HeightLSCnt=32, IntStatus0=0x3（因CSIBDG尺寸不匹配丢帧）

### 突破性测试：完整 1080p60 初始化运行

首次通过 Python API 启动完整 1080p60 模式：

```
Camera(1920, 1080, fps=60)
stSnsrMode ... 1920x1080 60.000000
OS04A10 1080P 60fps 12bit LINE Init OK!  ✅ ← 传感器初始化通过
[SAMPLE_PLAT_VI_INIT]-131: PLAT_INIT: StartSensor OK
[SAMPLE_PLAT_VI_INIT]-137: PLAT_INIT: StartDev[0]
[SAMPLE_PLAT_VI_INIT]-148: PLAT_INIT: StartMIPI OK  ✅ ← MIPI RX 配置通过！
[SAMPLE_PLAT_VI_INIT]-162: PLAT_INIT: SensorProbe OK
[SAMPLE_PLAT_VI_INIT]-210: PLAT_INIT: CreatePipe done
[SAMPLE_PLAT_VI_INIT]-218: PLAT_INIT: CreateIsp OK  ✅ ← ISP 初始化通过！
[SAMPLE_PLAT_VI_INIT]-220: PLAT_INIT: StartViChn
```

**MIPI RX 完全正常工作** — 不再出现 `decode: unknown`！

但 StartViChn 失败：
```
VI_SDK_IOC_S_CTRL - vi_sdk_enable_chn NG, No buffer space available
Trigger signal, code:SIGSEGV(11)!
```

### 进展分析

之前出现 `decode: unknown` 的旧测试 vs 本次成功通过 MIPI RX 阶段的唯一代码变更：

1. **`snsr_type_name[]` 数组修复**（缺失逗号 + OV2685 条目缺失 + NULL 防护）
2. **枚举和映射表正确添加**

旧测试中 `sns_type` 查找错误导致 `SAMPLE_COMM_SNS_GetSize()` 可能返回了错误的 `PIC_SIZE`，进而使 `mmf_vi_init_v2()` 传入了错误的尺寸，间接导致 MIPI RX 配置异常。

修复后的代码流：
```
解析ini获取sns_type=136 → GetSizeBySensor → PIC_1080P → GetPicSize → 1920×1080
  → StartSensor（I2C写入302+寄存器）
  → StartMIPI（ResetSensor→ResetMipi→SetMipiAttr→EnableClock→UnresetSensor）
  → SensorProbe（读ID验证）
  → CreatePipe
  → CreateIsp（SetPubAttr, Init, 加载bin, ISP_Run, cmos_init）
  → StartViChn ⚡ 失败：No buffer space available
```

### 当前问题：VB 缓冲池空间不足

`vi_sdk_enable_chn NG, No buffer space available` 出现在 `StartViChn` 阶段，表明：

1. VI_DMA_BUF 池大小为 `8945664` 字节（固定值，与 1440p30 相同）
2. 1920×1080 NV12 帧大小：`1920 × 1080 × 1.5 = 3,110,400` 字节
3. Pool 可容纳约 2.87 帧 → 足够 2 个缓冲
4. 但 `vi_sdk_enable_chn` 仍报告空间不足

可能原因：
- ISP 输出/VI 通道请求了不同 stride 计算方式
- SBS（Side-by-Side）或压缩模式需要额外空间
- "No buffer space available" 实际含义可能是池中无空闲槽位（已被占用或使用计数不对）
- VB 池创建时的 `u32BlkSize` 计算与 VI 通道使能时的期望不一致

### MIPI RX 完整寄存器扫描数据

#### MAC0_TOP (sensor mac) 非零寄存器
```
0x0A0C2000: 0x00006E71  ← CSI mode, VS/HS invert, CSI enable
0x0A0C2010: 0x00000020  ← TTL_IP_EN=0
0x0A0C2030: 0x00000020
0x0A0C2048: 0x00000001
0x0A0C2054: 0x00040000
0x0A0C2058: 0x00040000
0x0A0C2074: 0x76543210  ← TTL pinmux
0x0A0C20B0: 0x0FFF0FFF  ← CROP start
0x0A0C20B4: 0x0FFF0FFF  ← CROP end
0x0A0C20BC: 0x0000FFFF
0x0A0C20C0: 0xFFFFFFFF
0x0A0C20D0: 0x00020200
0x0A0C20D4: 0x00000FFF
0x0A0C20D8: 0x0AB00000  ← SubLVDS sync codes
0x0A0C20DC-0x0A0C20FC: sync codes
0x0A0C2100-0x0A0C2124: more sync codes
```

#### MAC0_CSI (CSI控制器) 非零寄存器
```
0x0A0C2400: 0x00000003  ← CSI_LANE_MODE: 4-lane, ECC on, no VC check
0x0A0C2404: 0x000C0000  ← HDR_EN=0, HDR_MODE=0, ID_RM_ELSE=1, ID_RM_OB=1
0x0A0C2408: 0x02220221  ← ECC/CRC control values
0x0A0C240C: 0x02310241
0x0A0C2410: 0x02510232
0x0A0C2414: 0x00002037  ← Short packet capture
0x0A0C2418: 0x00003210  ← VC mapping
0x0A0C241C: 0x02520242  ← More timing/calibration
0x0A0C2420: 0x02540244
0x0A0C2424: 0x02340224
0x0A0C2440: 0x00100001  ← CSI_DECODE=RAW12, ECC_NO_ERROR
0x0A0C2470: 0x00000F00  ← VS_GEN_MODE=0, reserved bits=0x0F
```

#### PHY_WRAP_4L 非零寄存器
```
0x0A0D0304: 0x00000413  ← D0_SEL=3, D1_SEL=1, D2_SEL=4
0x0A0D0308: 0x00000F12  ← D3_SEL=2, CK_SEL=1, CK_PNSWAP=1
0x0A0D030C: 0x00FF000F  ← DESKEW_LANE_EN
0x0A0D0310: 0x00000208  ← AUTO_IGNORE=0, AUTO_SYNC=0
0x0A0D0320: 0x0FFF0009
0x0A0D0334: 0x06000CAA
0x0A0D0338: 0x000000FF
0x0A0D0390: 0x00000021  ← HS settle state
0x0A0D03A4: 0x60202003  ← Per-lane DPHY timing (includes T_HS_SETTLE)
0x0A0D03A8: 0x00000020
0x0A0D0400-0x0A0D04EC: Per-lane deskew/calibration data (4 identical blocks)
```

### 关键结论

1. **`decode: unknown` 已被修复** — 根因是 `snsr_type_name[]` 数组断裂导致传感器类型查找错误，不是 MIPI 硬件问题
2. **新问题出现在 `StartViChn` 阶段的 VB 缓冲池分配** — `No buffer space available`
3. MIPI RX 所有寄存器、CSI 控制器、PHY 配置均正常
4. 需要分析 VB 池初始化流程，确认 `u32BlkSize` 计算是否与 VI 通道期望一致

### VB 池耗尽根因分析

`vi_enable_chn()`（内核 `vi_sdk_layer.c:449`）的预排队机制导致双重初始化时 VB 块不足：

```c
for (j = 0; j < num_buffers; j++) {   // num_buffers = CVI_VI_CHN_0_BUF = 2
    rc = vi_sdk_qbuf(chn);             // 从 VB Pool 取一块，失败返回 -ENOMEM
}
```

Pool[0] 只有 3 块，第一次 init 预排队用掉 2 块，第二次 init 需要再 2 块 → 只剩 1 块 → `CVI_ERR_VB_NOBUF`。

**`vi_disable_chn()`**（`vi_sdk_layer.c:537`）调用 `base_mod_jobs_exit()` 释放所有 VB 块。
在 `mmf_vi_init_v2` 前调用 `CVI_VI_DisableChn(0, 0)` 即可释放旧通道缓冲。

#### 修复

已在 `maix_camera_mmf.cpp:758` 添加最小分支：

```cpp
if (sensor_cfg.sns_type == OV_OS04A10_MIPI_4M_1080P60_12BIT) {
    CVI_VI_DisableChn(0, 0);
}
```

### Crop 区域修正（最终关键修复）

OS04A10 内部 ISP 会从 crop 区域自动裁剪一定数量的光学黑行/列，使得 crop 区域比输出尺寸大 16 行/列。

**1440p30 的配置：**
- Crop 区域: (0,0) → (2703,1535) = 2704×1536
- 输出尺寸: 2688×1520
- 差值: 16 行 + 16 列

**1080p60 原始配置（失败）：**
- Crop 区域: (384,220) → (2303,1299) = 1920×1080
- 输出尺寸: 1920×1080
- 差值: 0 → 传感器内部裁剪后仅输出 1064 行 → CSI 桥 HeightLSCnt

**1080p60 修复后配置（成功）：**
- Crop 区域: (384,220) → (2319,1315) = 1936×1096
- 输出尺寸: 1920×1080
- 差值: 16 行 + 16 列

修改文件: `os04a10_sensor_ctl.c` 中 1080p60 init 函数的 crop end 寄存器:
```
0x3804-0x3805: 0x090F (= 2319, 原 0x08FF = 2303)
0x3806-0x3807: 0x0523 (= 1315, 原 0x0513 = 1299)
```

### 最终验证结果（2026-07-11）

```
=== TEST 1080p60 (Camera(1920, 1080, fps=60)) ===

VIOutImgWidth     :1920
VIOutImgHeight    :1080
VIInImgWidth      :1920
VIInImgHeight     :1080
VIFPS             :  29          ← 帧正常抵达
VISofCh0Cnt       : 218          ← MIPI RX 接收帧起始
VIPreFECh0Cnt     : 217          ← 217/218 帧到达 ISP FrontEnd
VICsiIntStatus0   :0x6           ← VSYNC + TRIG，无帧丢弃！
VICsiCh0HeightGTCnt:   0         ← 无高度过大错误
VICsiCh0HeightLSCnt:   0         ← 无高度不足错误
VICsiCh0WidthGTCnt :   0         ← 无宽度过大错误
VICsiCh0WidthLSCnt :   0         ← 无宽度不足错误
✅ 1080p60 模式通过完整 VI → ISP 管线！
```

### VTS 被 default_reg_init + ISP AE 双重覆盖（第四轮修复）

**发现**：`cmos_fps_set(60)` 被正确调用，VTS 设为 1216，但 `cmos_fps_set(30)` 在之后被调用将 VTS 改回 2432。

**根因链**：
1. `sensor_global_init()`: `au32FL[0] = 2432`（1440p30的VTS）
2. `cmos_set_image_mode()`: 将 `u8ImgMode` 改为 1080p60，但 `au32FL[0]` 未被更新
3. `os04a10_init()`: init 函数写 VTS=1216，但 `default_reg_init()` 从 `astI2cData` 取残留值覆盖为 0
4. ISP AE 启动后从 `au32FL[0]`(=2432) 计算 VTS → 覆盖传感器为 2432 → VIFPS=29

**修复**：在 `cmos_set_image_mode` 的 `mode_set` 分支中添加：
```c
pstSnsState->au32FL[0] = g_astOs04a10_mode[u8SensorImageMode].u32VtsDef;
pstSnsState->au32FL[1] = ...;
pstSnsState->u32FLStd   = ...;
```

文件：`os04a10_cmos.c:1250-1254`

### 最终状态（已验证）

```
VTS readback 在 streaming 期间: 0x04C0 = 1216 ✓
VTS readback 在 cam.close() 之后: 0x0980 = 2432 (cleanup 调用 cmos_fps_set(30))
VIDevFPS: 29, VIFPS: 28-29
VICsiIntStatus0: 0x6 (VSYNC + TRIG, 无 FRAME_DROP)
VICsiCh0HeightLSCnt: 0
VICsiCh0WidthLSCnt:  0
```

**传感器运行在 60fps，但管线输出 ~29fps。**

### VPSS 帧率限制——v1 API 修正 + VPSS_FPS 环境变量覆盖（第五轮修复）

**v1 API 修正** (`sophgo_middleware.c:457`): `mmf_add_vi_channel` 新增 `int fps` 参数，移除硬编码 `fps=30`。该 API 仅被 `sample_vio.c` 使用（非运行时路径）。

**运行时路径分析**：
- `maix_camera_mmf.cpp` → `mmf_add_vi_channel_v2(_fps=60, ...)` → `mmf_add_vi_channel0`（预编译 `libmaixcam_lib.so`）
- 预编译 lib 通过 `SAMPLE_COMM_VPSS_Init`（动态链接，来自开源 `libsophgo-middleware.so`）初始化 VPSS
- 预编译 lib 内部构建 VPSS attr，fps 来源未知（无法修改预编译库）

**VPSS_FPS 环境变量覆盖**（`sample_common_vpss.c:26-39`）：
```c
const char *env_fps = getenv("VPSS_FPS");
if (env_fps) { int force = atoi(env_fps);
    if (force > 0) // 强制覆盖所有 VPSS 通道 fps }
```
运行时执行 `VPSS_FPS=60 python app.py` 即可生效，覆盖预编译 lib 传入的任意帧率。

### 修复总结

| 问题 | 根因 | 修复 | 文件 |
|------|------|------|------|
| `decode: unknown` | `snsr_type_name[]` 数组断裂 | 添加缺失逗号+OV2685条目+NULL防护 | `sample_common_sensor.c` |
| `No buffer space available` | VB Pool双重初始化耗尽 | 在`mmf_vi_init_v2`前`DisableChn` | `maix_camera_mmf.cpp:758` |
| `HeightLSCnt`帧高度不足 | crop=output，传感器内部裁剪16行 | crop区域扩大16行 | `os04a10_sensor_ctl.c:1419-1422` |
| `VIFPS=29`（60目标） | AE从 `au32FL[0]`(=2432) 计算VTS | `cmos_set_image_mode` 中更新 FL | `os04a10_cmos.c:1250-1254` |
| `VIFPS=29`（硬件限制） | ISP 吞吐约110M px/s < 124M px/s (1080p60) | 增加 `VPSS_FPS` 环境变量调试 | `sample_common_vpss.c:26-39` |
| v1 API fps 硬编码 | `mmf_add_vi_channel` 中固定 fps=30 | 新增 `int fps` 参数 | `sophgo_middleware.c:457` |
