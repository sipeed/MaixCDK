#include "dragonfly.h"

#include "tof_adapter.hpp"
// #include "cmap_jet.hpp"

#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#pragma GCC diagnostic ignored "-Wsign-compare"

#define VERSION "1.5.0.0"

#define delay_ms(ms) bflb_platform_delay_ms(ms)

DragonflyISPInitSet isp_init_param_;
DragonflyBiningModeSet roi_set_;
DragonflyAESetSpi ae_set_spi_;
DragonflyFrameHead frame_head_;
DragonflyFrameTail frame_tail_;

AntiMMI antimmi_config_;
LensCoeff lens_coeff_;

/*******************************************************************************
 * Function Name  : char * DragGetVersion()
 * Description    : Get version of SDK
 * Input          : None
 * Return         : char *version
 *******************************************************************************/
char *DragGetVersion() {
  char *version = (char *)VERSION;
  return version;
}

/*******************************************************************************
 * Function Name  : int  DragISPBooting()
 * Description    : Booting up isp
 * Input          : None
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int DragISPBooting() {
  int ret = 0;
  int count = 1;
  uint32_t id = 0;

  uint32_t adr_val[] = {DRAG_DAT_REG, DRAG_DAT_REG, DRAG_DAT_REG, 0x4000e000,
                        0x4000e004,   0x4000e008,   0x4000e010};
  uint32_t buf_reg[] = {0x00000000, 0x00000000, 0x00000000, 0x80080082,
                        0x01ffff00, 0x0000000f, 0xffffffff};

  DragSwReset();  //可以暂时注释掉避免看太多波形
  delay_ms(5);

  do {
    SPII2CRegRd(0x00000000, &id);
    if (id != 0x2000fc00) {
      if (count == 6) {
        eprintln("ERROR: id = %x, not equal 0x2000fc00\n", id);
        return -1;
      }
    } else {
      break;
    }
  } while (count++);

  for (int j = 0; j < 7; j++) {
    ret = SPII2CRegWr(adr_val[j], buf_reg[j]);
  }

  count = 0;
  while (count < 5) {
    ret = SPII2CCheckISPIsIDL();
    if (ret) {
      count++;
      delay_ms(100);
    } else {
      break;
    }
  }
  delay_ms(10);

  return ret;
}

/*******************************************************************************
 * Function Name  : int  DragISPInit()
 * Description    : Send initialization parameters to the ISP
 * Input          : None
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int DragISPInit() {
  int ret = 0;

  if (isp_init_param_.fps > 20 || isp_init_param_.out_if > DRAG_MIPI_OUT ||
      isp_init_param_.roi_ul_x > ROI_MAX_VALUE ||
      isp_init_param_.roi_ul_y > ROI_MAX_VALUE ||
      isp_init_param_.roi_br_x > ROI_MAX_VALUE ||
      isp_init_param_.roi_br_y > ROI_MAX_VALUE ||
      isp_init_param_.roi_ul_x > isp_init_param_.roi_br_x ||
      isp_init_param_.roi_ul_y > isp_init_param_.roi_br_y ||
      isp_init_param_.binning_mode > DRAG_BINNING_MODE2 ||
      isp_init_param_.uart_bps > DRAG_UART_921600) {
    eprintln("ERROR: Initial parameter is invalid\n");
    return -1;
  }
  isp_init_param_.ap_confirm = false;

  //    if(spi_device_ || i2c_device_)
  //    {
  // ae_set_spi_.ae_en = 1;
  // ae_set_spi_.exposure_time = 20000;
  ret = SPII2CSetCmdValue(DRAG_8082_AE_SET, (uint32_t *)&ae_set_spi_,
                          sizeof(ae_set_spi_) / 4);
  ret = SPII2CSetCmdValue(DRAG_8080_ISP_INIT_SET, (uint32_t *)&isp_init_param_,
                          sizeof(isp_init_param_) / 4);
  if (!ret) {
    isp_init_param_.ap_confirm = true;
    ret =
        SPII2CSetCmdValue(DRAG_8080_ISP_INIT_SET, (uint32_t *)&isp_init_param_,
                          sizeof(isp_init_param_) / 4);
  }
  return ret;
}

/*******************************************************************************
 * Function Name  : int  DragSetISPStart()
 * Description    : ISP start to transfer frame data
 * Input          : None
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int DragSetISPStart() {
  int ret = 0;
  uint32_t cmdval = true;
  ret = SPII2CSetCmdValue(DRAG_8081_ISP_START_STOP, &cmdval, 1);
  return ret;
}

/*******************************************************************************
 * Function Name  : DragSetISPStop()
 * Description    : ISP stop to transfer frame data
 * Input          : None
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int DragSetISPStop() {
  int ret = 0;
  uint32_t cmdval = false;
  ret = SPII2CSetCmdValue(DRAG_8081_ISP_START_STOP, &cmdval, 1);
  return ret;
}

/*******************************************************************************
 * Function Name  : int  DragSwReset()
 * Description    : Software reset isp
 * Input          : None
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int DragSwReset() {
  int ret;
  uint32_t adr_val[] = {DRAG_DAT_REG, DRAG_DAT_REG, DRAG_DAT_REG, 0x4000e010,
                        0x4000e00c,   0x4000e00c,   0x4000e00c};
  uint32_t buf_reg[] = {0x00000000, 0x00000000, 0x00000000, 0xfffffffe,
                        0x00000000, 0x00000001, 0x00000000};
  for (int j = 0; j < 7; j++) {
    ret = SPII2CRegWr(adr_val[j], buf_reg[j]);
  }
  return ret;
}

/*******************************************************************************
 * Function Name  : DragSetAntiMMI(AntiMMI *antimmi_config)
 * Description    : Set the Anti MMI to avoid interference between devices
 * Input          : AntiMMI &antimmi_config
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int DragSetAntiMMI(AntiMMI *antimmi_config) {
  int ret = 0;
  if (antimmi_config->mode > DRAG_AntiMMI_MANUAL_MODE) {
    eprintln("ERROR: Invalid Anti MMI mode\n");
    return -1;
  }
  ret =
      SPII2CSetCmdValue(DRAG_8019_ANTIMMI_SET, (uint32_t *)&antimmi_config, 1);

  return ret;
}

/*******************************************************************************
 * Function Name  : DragGetLensCoeff(LensCoeff *cali_data)
 * Description    : Get Lens Coeff of module
 * Input          : LensCoeff &cali_data
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int DragGetLensCoeff(LensCoeff *cali_data) {
  int ret;
  uint32_t size;
  size = sizeof(LensCoeff);
  if (size % 4 != 0) size += 3;
  size = size / 4;
  if (size > 11) size = 11;

  uint32_t cali_buf[12];
  ret = SPII2CGetCmdValue(DRAG_9003_SENSOR_INFO_GET, cali_buf, size);
  if (ret) return ret;

  LensCoeff *pcoff = (LensCoeff *)cali_buf;
  cali_data->cali_mode = pcoff->cali_mode;
  cali_data->fx = pcoff->fx;
  cali_data->fy = pcoff->fy;
  cali_data->u0 = pcoff->u0;
  cali_data->v0 = pcoff->v0;
  cali_data->k1 = pcoff->k1;
  cali_data->k2 = pcoff->k2;
  cali_data->k3 = pcoff->k3;
  cali_data->k4_p1 = pcoff->k4_p1;
  cali_data->k5_p2 = pcoff->k5_p2;
  cali_data->skew = pcoff->skew;

  return ret;
}

/*******************************************************************************
 * Function Name  : int  SPII2CCheckISPIsIDL()
 * Description    : Check if ISP is idle
 * Input          : None
 * Return         : -1: busy
 *                  0: idle
 *******************************************************************************/
int SPII2CCheckISPIsIDL() {
  uint32_t status;
  uint32_t count = 0;
  while (1) {
    SPII2CRegRd(DRAG_IDLE_REG, &status);
    switch (status) {
      case IDEL_STATE:
        return 0;
        break;
      case BUSY_STATE:
        count++;
        break;
      default:
        count++;
        break;
    }
    delay_ms(5);
    if (count > 400) return -1;
  }
}

/*******************************************************************************
 * Function Name  : int  SPII2CCheckISPCmdStatus(uint32_t cmd)
 * Description    : Check that the ISP command wether be sent successfully
 * Input          : uint32_t cmd
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int SPII2CCheckISPCmdStatus(uint32_t cmd) {
  int ret;
  uint32_t cmd_status;
  uint16_t err_code;
  ret = SPII2CCheckISPIsIDL();
  if (ret != 0)
    return -1;
  else {
    delay_ms(1);
    SPII2CRegRd(DRAG_STA_REG, &cmd_status);
    err_code = (uint16_t)cmd_status;
    if ((cmd & 0x0000ff00) == 0x8000) {
      err_code = (uint16_t)cmd_status;
    } else {
      err_code = (uint16_t)(cmd_status & 0x00ff);
    }
    if ((cmd_status >> 16) == cmd && !err_code) {
      return 0;
    } else {
      return -1;
    }
  }
}

/*******************************************************************************
 * Function Name  : uint8_t  SPII2CGetCheckSum(MsgHead *msg)
 * Description    : Calculate the checksum
 * Input          : MsgHead *msg
 * Return         : checksum
 *******************************************************************************/
uint8_t SPII2CGetCheckSum(MsgHead *msg) {
  uint8_t sum = 0;
  msg->checksum = 0;
  uint8_t *ptr = (uint8_t *)msg;
  size_t i;
  for (i = 0; i < msg->size; i++) {
    sum += *ptr++;
  }

  return sum;
}

/*******************************************************************************
 * Function Name  : int  SPII2CGetCmdValue(uint32_t cmd, uint32_t *value,
 *                  uint32_t num)
 * Description    : Get the value of the relevant parameter with the command
 * Input          : uint32_t cmd, uint32_t *value, uint32_t num
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int SPII2CGetCmdValue(uint32_t cmd, uint32_t *value, uint32_t num)
{
    int ret;
    int count = 0;
    uint32_t rx[20];
start:
    SPII2CSetCmdValue(cmd, value, num);
    // usleep(1000 * 10);
    delay_ms(10);
    ret = SPII2CCheckISPCmdStatus(cmd);
    if(ret != 0)
    {
        if(count++ > 5)
        {
            eprintln("ERROR: write cmd: %x failed!\n", cmd);
            return ret;
        } else
        {
            goto start;
        }
    }
    SPII2CMultipleRegRd(DRAG_DAT_REG, rx, num + 2);
    for(uint32_t i = 0; i < num; i++)
    {
        value[i] = rx[i + 2];
    }
    return ret;
}

/*******************************************************************************
 * Function Name  : int  SPII2CSetCmdValue(uint32_t cmd, uint32_t *value,
 *                  uint32_t num)
 * Description    : Set the value of the relevant parameter with the command
 * Input          : uint32_t cmd, uint32_t *value, uint32_t num
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int SPII2CSetCmdValue(uint32_t cmd, uint32_t *value, uint32_t num) {
  int ret = 0;
  int count = 0;
start:
  if (SPII2CCheckISPIsIDL()) {
    eprintln("ISP is busy\n");
  }
  MsgBody msg;
  msg.msg_head.size = sizeof(MsgHead) + sizeof(uint32_t) * num;
  msg.msg_head.cmd = cmd;
  msg.msg_head.cam_id = 1;
  uint32_t i;

  for (i = 0; i < num; i++) msg.buffer[i] = value[i];
  msg.msg_head.checksum = SPII2CGetCheckSum(&msg.msg_head);
  SPII2CMultipleRegWr(DRAG_DAT_REG, (uint32_t *)&msg, msg.msg_head.size / 4);
  delay_ms(10);
  ret = SPII2CCheckISPCmdStatus(cmd);
  if (ret != 0) {
    if (count++ > 10) {
      eprintln("ERROR: write cmd: %x failed!\n", cmd);
      return ret;
    } else {
      goto start;
    }
  }

  return ret;
}

/*******************************************************************************
 * Function Name  : int  SPII2CRegRd(uint32_t addr, uint32_t *data)
 * Description    : Read one byte data
 * Input          : uint32_t addr, uint32_t *data
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int SPII2CRegRd(uint32_t addr, uint32_t *data) {
  int ret = -1;
  uint32_t tx_data[] = {
      DRAG_RD, addr, DUMMY, DUMMY,
      DUMMY  // get data here
  };
  uint32_t rx_data[5] = {0};

  gpio_write(TOF_CS_PIN, 0);
  ret = spi_transmit_receive(g_spi, tx_data, rx_data, 5, 3);
  gpio_write(TOF_CS_PIN, 1);

  *data = rx_data[4];
  return ret;
}

/*******************************************************************************
 * Function Name  : int  SPII2CRegWr(uint32_t addr, uint32_t data)
 * Description    : Write one byte data
 * Input          : uint32_t addr, uint32_t *data
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int SPII2CRegWr(uint32_t addr, uint32_t data) {
  int ret = -1;
  uint32_t tx_data[] = {
      DRAG_WR,
      addr,
      data,
      DUMMY,
  };

  gpio_write(TOF_CS_PIN, 0);
  ret = spi_transmit(g_spi, tx_data, 4, 3);
  gpio_write(TOF_CS_PIN, 1);

  return ret;
}

/*******************************************************************************
 * Function Name  : int  SPII2CMultipleRegRd(uint32_t addr,
 *                  uint32_t *data, uint32_t len)
 * Description    : write len byte data
 * Input          : uint32_t addr, uint32_t *data, uint32_t len
 *                  value of 'len' need littler than 298 byte
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int SPII2CMultipleRegRd(uint32_t addr, uint32_t *data, uint32_t len) {
  int ret = -1;
  uint32_t tx_data_head[] = {DRAG_RD, addr, DUMMY, DUMMY};
  uint32_t *tx_data_buf = (uint32_t *)malloc((len + 4) * sizeof(uint32_t));
  uint32_t *rx_data_buf = (uint32_t *)malloc((len + 4) * sizeof(uint32_t));
  tx_data_buf[0] = tx_data_head[0];
  tx_data_buf[1] = tx_data_head[1];
  tx_data_buf[2] = tx_data_head[2];
  tx_data_buf[3] = tx_data_head[3];
  arch_memset(tx_data_buf + 4, 0xff, len * sizeof(uint32_t));

  gpio_write(TOF_CS_PIN, 0);
  ret = spi_transmit_receive(g_spi, tx_data_buf, rx_data_buf, len + 4, 3);
  gpio_write(TOF_CS_PIN, 1);

  arch_memcpy_fast(data, rx_data_buf + 4, len * sizeof(uint32_t));
  free(rx_data_buf);
  free(tx_data_buf);

  return ret;
}

/*******************************************************************************
 * Function Name  : int  SPII2CMultipleRegWr(uint32_t addr,
 *                  uint32_t *data, uint32_t len)
 * Description    : write len byte data
 * Input          : uint32_t addr, uint32_t *data, uint32_t len
 *                  value of 'len' need littler than 298 byte
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int SPII2CMultipleRegWr(uint32_t addr, uint32_t *data, uint32_t len) {
  int ret = -1;
  uint32_t tx_data_head[] = {DRAG_WR, addr};
  uint32_t tx_data_tail[] = {DUMMY};

  uint32_t *tx_data_buf = (uint32_t *)malloc((len + 3) * sizeof(uint32_t));
  tx_data_buf[0] = tx_data_head[0];
  tx_data_buf[1] = tx_data_head[1];
  arch_memcpy_fast(tx_data_buf + 2, data, len * 4);
  tx_data_buf[len + 2] = tx_data_tail[0];

  gpio_write(TOF_CS_PIN, 0);
  ret = spi_transmit(g_spi, tx_data_buf, len + 3, 3);
  gpio_write(TOF_CS_PIN, 1);

  free(tx_data_buf);
  return ret;
}

/*******************************************************************************
 * Function Name  : int  SPIReadBurstData(uint32_t src_addr,
 *                  uint32_t *dst_addr, uint32_t len)
 * Description    : Read large numble of SPI data
 * Input          : uint32_t src_addr, uint32_t *dst_addr, uint32_t len
 * Return         : -1: failed
 *                  0: succeed
 *******************************************************************************/
int SPII2CBurstDataRead(uint32_t src_addr, uint32_t *dst_addr, uint32_t len) {
  int ret = 0;
  uint32_t remain_lenth = 0;
  if (len > 1024 * 1024) {
    eprintln("ERROR: too big lenth!\n");
    return -1;
  }

  while (len > 0) {
    if (len > SPI_BURST_READ_LENTH) {
      ret = SPII2CMultipleRegRd(src_addr, dst_addr, SPI_BURST_READ_LENTH / 4);
      if (ret < 0) {
        eprintln("ERROR: SPII2CMultipleRegRd frame data failed\n");
        return -1;
      }
      len -= SPI_BURST_READ_LENTH;
      dst_addr += SPI_BURST_READ_LENTH / 4;
      src_addr += SPI_BURST_READ_LENTH;
    } else {
      if (len % 4 != 0)
        remain_lenth = len / 4 + 1;
      else
        remain_lenth = len / 4;

      ret = SPII2CMultipleRegRd(src_addr, dst_addr, remain_lenth);
      if (ret < 0) {
        eprintln("ERROR: SPII2CMultipleRegRd frame data failed\n");
        return -1;
      }
      len = 0;
    }
  }

  return 0;
}

void msl_setup(int fps, BinningMode mode, uint8_t exposure)
{
  isp_init_param_.fps = fps;
  isp_init_param_.out_if = DRAG_SPI_OUT;
  isp_init_param_.out_mode = DRAG_DEPTH_ONLY;
  isp_init_param_.roi_ul_x = 0;
  isp_init_param_.roi_ul_y = 0;
  isp_init_param_.roi_br_x = 99;
  isp_init_param_.roi_br_y = 99;
  isp_init_param_.binning_mode = mode;
  isp_init_param_.uart_bps = DRAG_UART_115200;

  roi_set_.roi_binning_en = DRAG_BINNING_SET;
  roi_set_.binning_mode = isp_init_param_.binning_mode;
  roi_set_.roi_ul_x = isp_init_param_.roi_ul_x;
  roi_set_.roi_ul_y = isp_init_param_.roi_ul_y;
  roi_set_.roi_br_x = isp_init_param_.roi_br_x;
  roi_set_.roi_br_y = isp_init_param_.roi_br_y;

  ae_set_spi_.ae_en = exposure;
  ae_set_spi_.exposure_time = exposure;

  antimmi_config_.mode = 0;
  SPII2CSetCmdValue(DRAG_8019_ANTIMMI_SET, (uint32_t *)&antimmi_config_, 1);
}

int spi_init(int id)
{
  return tof_init(id);
}