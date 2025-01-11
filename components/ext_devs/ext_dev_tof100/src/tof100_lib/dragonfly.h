#pragma once

#include <stdbool.h>
#include <stdint.h>

#define DUMMY 0xffffffff

#define DRAG_WR 0x00000000
#define DRAG_RD 0x00000001

#define DRAG_I2C_ADDR 0x53

#define DRAG_IDLE_REG 0x40003004
#define DRAG_STA_REG 0x40003000
#define DRAG_DAT_REG 0x40007000

#define DATA_BASE_ADDRESS 0x20000000
#define DATA_OFFSET_ADDRESS 0xE3AC
#define DATA_ADDRESS_LENGTH 40000
#define DATA_MIN_LENGTH 1250
#define DATA_HEAD_LENGTH 20
#define DATA_TAIL_LENGTH 2
#define DATA_HEAD_INFO_LENGTH 16

#define IDEL_STATE 0x12341234
#define BUSY_STATE 0x8008ffff

#define PACKAGE_REC_HEAD1 0xCC
#define PACKAGE_REC_HEAD2 0xA0
#define PACKAGE_REC_END 0xDD

/* UART transfer package define */
#define UART_PER_SEND_SIZE 256
#define UART_DATA_INFO_LENGTH 7

#define UART_SEND_HEAD1 0xAA
#define UART_SEND_HEAD2 0xA0
#define UART_SEND_END 0xBB
#define UART_REC_HEAD1 0xCC
#define UART_REC_HEAD2 0xA0
#define UART_REC_END 0xDD

/* SPI transfer package define */
#define SPI_BURST_PER_READ_SIZE 4096
#define SPI_BURST_READ_LENTH (SPI_BURST_PER_READ_SIZE - DATA_HEAD_INFO_LENGTH)

#define ROI_MINI_VALUE 0
#define ROI_MAX_VALUE 99

enum DragCmd {
  DRAG_10_UART_MSG_CMD_CRC_ERR = 0x10,
  DRAG_20_UART_MSG_FLASH_INIT = 0x20,
  DRAG_21_UART_MSG_FLASH_EARSE,
  DRAG_22_UART_MSG_FLASH_WRITE,
  DRAG_23_UART_MSG_FLASH_READ,
  DRAG_24_UART_MSG_FLASH_END,
  DRAG_25_UART_MSG_FLASH_REBOOT,
  DRAG_26_UART_MSG_FLASH_OK,
  DRAG_27_UART_MSG_FLASH_JUMP,
  DRAG_80_ISP_INIT_SET = 0x80,
  DRAG_81_ISP_START_STOP,
  DRAG_82_AE_SET,
  DRAG_83_ROI_BINNING_SET,
  DRAG_84_ISP_INIT_SET_SOLIDIFY,
  DRAG_85_FPS_SET,
  DRAG_86_UART_BPS_SET,
  DRAG_87_IR_LIMIT_SET,
  DRAG_88_AP_SYNC_SWITCH,
  DRAG_89_ANTIMMI_SET,
  DRAG_8A_IR_LIMIT_GET,
  DRAG_8B_FPS_GET,
  DRAG_8C_SOLIDIFY_CONFIG_GET,
  DRAG_8D_SOLIDIFY_CONFIG_ERASURE,
  DRAG_8E_ANTIMMI_GET,
  DRAG_90_SENSOR_INFO_GET = 0x90,
  DRAG_91_AE_WINDOW_SET,
  DRAG_92_AE_WINDOW_GET,
  DRAG_9A_OBST_DETECT_TRIGGER = 0x9A,
  DRAG_9B_OBST_REF_PLANE_COLLECT,
  DRAG_FF_FRAME_DATA = 0xFF,
  DRAG_800F_AE_WINDOW_SET = 0x800F,
  DRAG_8019_ANTIMMI_SET = 0x8019,
  DRAG_8080_ISP_INIT_SET = 0x8080,
  DRAG_8081_ISP_START_STOP,
  DRAG_8082_AE_SET,
  DRAG_8083_ROI_BINNING_SET,
  DRAG_8084_ISP_INIT_SET_SOLIDIFY,
  DRAG_8085_FPS_SET,
  DRAG_8086,
  DRAG_8087_IR_LIMIT_SET,
  DRAG_8088_AP_SYNC_SWITCH,
  DRAG_808D_SOLIDIFY_CONFIG_ERASURE = 0x808D,
  DRAG_9003_SENSOR_INFO_GET = 0x9003,
  DRAG_900A_FPS_GET = 0x900A,
  DRAG_9012_AE_WINDOW_GET = 0x9012,
  DRAG_9019_ANTIMMI_GET = 0x9019,
  DRAG_908A_IR_LIMIT_GET = 0x908A,
  DRAG_908C_SOLIDIFY_CONFIG_GET = 0x908C,
  DRAG_9090_OBST_DETECT_TRIGGER = 0x9090,
  DRAG_9091_OBST_REF_PLANE_COLLECT = 0x9091,
};

enum ROIBinningEn {
  DRAG_BINNING_ROI_DISABLE = 0,
  DRAG_ROI_SET,
  DRAG_BINNING_SET,
};

enum BinningMode {
  DRAG_BINNING_MODE0 = 0,
  DRAG_BINNING_MODE1,
  DRAG_BINNING_MODE2,
};

enum OutMode {
  DRAG_DEPTH_ONLY = 0,
  DRAG_DEPTH_AND_IR,
};

enum OutIF {
  DRAG_NO_OUT = 0,
  DRAG_UART_OUT,
  DRAG_SPI_OUT,
  DRAG_MIPI_OUT = 4,
};

enum UARTBPS {
  DRAG_UART_4800 = 0,
  DRAG_UART_9600,
  DRAG_UART_57600,
  DRAG_UART_115200,
  DRAG_UART_230400,
  DRAG_UART_460800,
  DRAG_UART_921600,
};

enum UARTMsgDealStatus {
  DRAG_NO_DEAL = -1,
  DRAG_DEAL_CMD,
  DRAG_DEAL_FRAME,
};

enum UartRecMux {
  UART_PACKAGE_HEAD1 = 0,
  UART_PACKAGE_HEAD2,
  UART_DATA_LEN_L,
  UART_DATA_LEN_H,
  UART_CMD,
  UART_DATA,
  UART_CHECKSUM,
  UART_PACKAGE_END,
};

enum AntiMMIMode {
  DRAG_AntiMMI_OFF = 0,
  DRAG_AntiMMI_AUTO_MODE,
  DRAG_AntiMMI_MANUAL_MODE,
};

enum ObstacleDetectMode {
  DRAG_DETECT_OFF = 0,
  DRAG_REF_PLANE_COLLECT_TRIGGER_MODE,
  DRAG_OBST_DETECT_MANUAL_TRIGGER_MODE,
  DRAG_OBST_DETECT_AUTO_TRIGGER_MODE,
  DRAG_ANGLE_DETECT_MANUAL_TRIGGER_MODE,
  DRAG_ANGLE_DETECT_AUTO_TRIGGER_MODE,
  DRAG_DEPTH_DATA_MANUAL_TRIGGER_MODE,
  DRAG_DEPTH_DATA_AUTO_TRIGGER_MODE,
};

typedef struct {
  uint8_t width;
  uint8_t height;
  uint8_t output_mode;
  uint16_t frame_id;
  uint8_t error_code;
  uint8_t temp_int;
  uint8_t temp_ext;
  uint32_t exposure_time;
  uint8_t isp_ver;
  uint16_t data_lenth;
  uint16_t depth_data_size;
  uint16_t ir_data_size;
  uint16_t *depth_data;
  uint16_t *ir_data;
  bool ready;
} __attribute__((packed)) DragonflyFrame;

typedef struct {
  uint8_t head1;
  uint8_t head2;
  uint8_t data_len0_7;
  uint8_t data_len8_15;
  uint8_t command;
  uint8_t output_mode;  //  0:Depth only, 1:Depth+IR
  uint8_t tempext;      //  reg08
  uint8_t tempint;      //  reg09
  uint8_t exposure_time0_7;
  uint8_t exposure_time8_11;
  uint8_t exposure_time12_19;
  uint8_t exposure_time20_23;
  uint8_t error_code;
  uint8_t reserved1;
  uint8_t frame_height;
  uint8_t frame_width;
  uint16_t frame_id;  // 12-bit frame ID, accumulates each frame, 0~4095
  uint8_t isp_veraion;
  uint8_t reserved3;
} __attribute__((packed)) DragonflyFrameHead;

typedef struct {
  uint8_t checksum;
  uint8_t end_mask;
  uint8_t reserved[2];
} __attribute__((packed)) DragonflyFrameTail;

typedef struct {
  uint32_t angle_x;
  uint32_t angle_y;
  uint16_t distance;
  uint16_t obs_result;
} __attribute__((packed)) DragonflyProjectorFrameBody;

typedef struct {
  uint8_t output_mode;
  uint16_t frame_id;
  uint8_t error_code;
  uint8_t temp_int;
  uint8_t temp_ext;
  uint32_t exposure_time;
  uint8_t isp_ver;
  uint16_t data_lenth;
  float angle_x;
  float angle_y;
  uint16_t distance;
  uint16_t obs_result;
  bool ready;
} __attribute__((packed)) DragonflyProjectorFrame;

typedef struct {
  uint8_t fps;     // set fps
  uint8_t out_if;  // Data output interface: 0-no output, 1-UART, 2-SPI, 4-MIPI
  uint8_t out_mode;  // Output mode setting: 0-output Depth, 1-output Depth+IR
  uint8_t roi_ul_x;  // X of the higher left coordinate of the ROI region
  uint8_t roi_ul_y;  // Y of the higher left coordinate of the ROI region
  uint8_t roi_br_x;  // X of the lower right coordinate of the ROI region
  uint8_t roi_br_y;  // Y of the lower right coordinate of the ROI region
  uint8_t binning_mode;  // Binning mode setting：
                         // 0-Binning closed,
                         // 1-Binning in the 2 by 2 region
                         // 2-Binning in the 4 by 4 region
  uint8_t
      uart_bps;  // Baud rate setting, baud rate table (0-7)：4800/9600/57600/
                 // 115200(default)/230400/460800/921600/1843200
  uint8_t reserved[2];  // Two bytes are reserved to keep four bytes aligned
  uint8_t ap_confirm;   // AP Confirm, 0：set，1：Confirm
} __attribute__((packed)) DragonflyISPInitSet;

typedef struct {
  uint8_t roi_binning_en;  // switch of ROI_Binning mode
  uint8_t roi_ul_x;        // X of the higher left coordinate of the ROI region
  uint8_t roi_ul_y;        // Y of the higher left coordinate of the ROI region
  uint8_t roi_br_x;        // X of the lower right coordinate of the ROI region
  uint8_t roi_br_y;        // Y of the lower right coordinate of the ROI region
  uint8_t binning_mode;    // Binning mode setting: 0-2
  uint8_t reserved[3];     // 3 bytes are reserved to keep 4 bytes aligned
} __attribute__((packed)) DragonflyBiningModeSet;

typedef struct {
  uint32_t ae_en;          // switch of AE mode
  uint32_t exposure_time;  // Exposure time setting
} __attribute__((packed)) DragonflyAESetSpi;

typedef struct {
  uint8_t ae_en;             // switch of AE mode
  uint8_t exposure_time[4];  // Exposure time setting
} __attribute__((packed)) DragonflyAESetUart;

typedef struct {
  uint32_t size;
  uint16_t cmd;
  uint8_t cam_id;
  uint8_t checksum;
} __attribute__((packed)) MsgHead;

typedef struct {
  MsgHead msg_head;
  uint32_t buffer[256];
} __attribute__((packed)) MsgBody;

typedef struct {
  uint8_t cmd;
  uint8_t status;
} __attribute__((packed)) CmdStatus;

typedef struct {
  uint8_t cali_mode;  // 0:Normal, 1:Fisheye
  uint32_t fx;        // fixpoint: u14p18
  uint32_t fy;        // fixpoint: u14p18
  uint32_t u0;        // fixpoint: u14p18
  uint32_t v0;        // fixpoint: u14p18
  uint32_t k1;        // fixpoint: s5p27
  uint32_t k2;        // fixpoint: s5p27
  uint32_t k3;        // fixpoint: s5p27
  uint32_t k4_p1;     // fixpoint: s5p27, normal mode is k4, fisheye mode is p1
  uint32_t k5_p2;     // fixpoint: s5p27, normal mode is k5 or unused, fisheye
                      // mode is p2
  uint32_t skew;      // fixpoint: s8p24
} __attribute__((packed)) LensCoeff;

typedef struct {
  uint8_t mode;   // 0: AntiMMI disabled, 1: auto mode 2: manual mode
  uint8_t devID;  // Set the devID when set manual mode
  uint16_t reserved;
} __attribute__((packed)) AntiMMI;

typedef struct {
  uint16_t x0;
  uint16_t y0;
  uint16_t x1;
  uint16_t y1;
} __attribute__((packed)) DragonflyAEWindow;

typedef struct {
  uint8_t mode;
  uint8_t manu_times;  // need in manual mode
  uint8_t reserved[2];
} __attribute__((packed)) ObstacleDetect;

typedef struct {
  uint32_t status;
  uint32_t valid_point;
} __attribute__((packed)) PlaneCollectResult;

enum MSL_BinningMode {
  MSL_BinningMode0_1x1 = 0,
  MSL_BinningMode1_2x2,
  MSL_BinningMode2_4x4,
};

int spi_init(int id);
void msl_setup(int fps, BinningMode mode, uint8_t exposure);
bool DragFrameSave(uint32_t *data, uint32_t size, uint16_t frame_id);

uint16_t DragCenterDepth(uint16_t *depth_data, uint8_t width, uint8_t height);

int DragISPInit();
int DragSetISPStart();
int DragSetISPStop();
int DragSetAEEn(bool flg);
int DragSetExposureTime(int times);
int DragSetROIAndBinningOff();
int DragSetROIRange(int x1, int y1, int x2, int y2);
int DragSetBiningMode(int mode);
int DragSetISPSolidifiedFlash();
int DragSetFPS(int fps);
int DragGetFPS(int *fps);
int DragSetUartBps(int uart_bps);
int DragSetIRLimit(int limit);
int DragGetIRLimit(int *limit);
int DragSetAPSync(bool flg);
int DragSetAntiMMI(AntiMMI *antimmi_config);
int DragGetAntiMMI(AntiMMI *antimmi_config);
int DragGetLensCoeff(LensCoeff *cali_data);
int DragGetCalib(float *calib_buf);
int DragGetISPSolidifiedConfig(DragonflyISPInitSet *solidified_config);
int DragErasureISPSolidifiedConfig();
int DragSetAEWindow(DragonflyAEWindow *ae_roi);
int DragGetAEWindow(DragonflyAEWindow *ae_roi);
int DragSetObstacleDetectTrigger(ObstacleDetect *obst_detect);
int DragGetObstacleDetectRefPlaneResult(PlaneCollectResult *plane_result);

int DragSwReset();
int DragISPBooting();
int DragStartDataMonitor();
int DragStopDataMonitor();
char *DragGetVersion();
int DragOTAWithSPII2C(const char *firm_name, int base_address);
int DragOTAWithUART(const char *firm_name);

int UartRecMsgDeal(uint8_t data);
int UartCheckISPCmdStatus(uint8_t cmd);
int UartGetISPCmdBack(uint8_t cmd, uint8_t *rx_data, uint32_t rx_size);
uint8_t UartGetCheckSum(uint8_t *data, uint32_t len);
int UartCMDWriteBurstDataCheck(unsigned int pWriteAddressData,
                               unsigned int *pWriteValueData,
                               unsigned int Lenth);
int UartCMDWriteReboot(void);
int UartCMDWriteInit(void);
int UartCMDWriteFlashEnd(void);
int UartCMDWriteJump(void);
int UartCMDWriteFlashOK(void);
int UartWriteFlashData(unsigned int pWriteAddressData,
                       unsigned int *pWriteValueData, unsigned int Lenth);
uint8_t SPII2CGetCheckSum(MsgHead *msg);
int SPII2CCheckISPIsIDL();
int SPII2CCheckISPCmdStatus(uint32_t cmd);
int SPII2CRegRd(uint32_t addr, uint32_t *data);
int SPII2CRegWr(uint32_t addr, uint32_t data);
int SPII2CMultipleRegRd(uint32_t addr, uint32_t *data, uint32_t len);
int SPII2CMultipleRegWr(uint32_t addr, uint32_t *data, uint32_t len);
int SPII2CMultipleRegWrOTA(uint32_t addr, uint32_t *data, uint32_t len);
int SPII2CBurstDataRead(uint32_t src_addr, uint32_t *dst_addr, uint32_t len);
int SPII2CSetCmdValue(uint32_t cmd, uint32_t *value, uint32_t num);
int SPII2CGetCmdValue(uint32_t cmd, uint32_t *value, uint32_t num);
int SPII2CWriteFlashStartCopy(void);
int SPII2CCheckFlashStartCopy(void);
int SPII2CReadFlashCMD(unsigned int start_addr, unsigned int end_addr,
                       unsigned int start_address);
int SPII2CCheckReadFlashCMD(unsigned int start_addr, unsigned int end_addr,
                            unsigned int start_address);
int SPII2CWriteFlashCMD(unsigned int start_addr, unsigned int end_addr,
                        unsigned int start_address);
int SPII2CCheckFlashCMD(unsigned int start_addr, unsigned int end_addr,
                        unsigned int start_address);
int DragHeadDecode();
//    int DragFrameDecode();
int DragProjFrameDecode();
int DragFrameTailDecode();
int DragSPII2CGrapOneFrame();
// static void DragGrapFrameThread(void *pParam);
