#ifndef __COMMON_HW_H__
#define __COMMON_HW_H__

#ifndef COMM_HW_PRT
#define COMM_HW_PRT(fmt...)   \
do {\
    printf("[COMM_FW][%s][%5d] ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)
#endif


AX_S8 COMMON_ISP_GetI2cDevNode(AX_U8 nDevId);

#endif //__COMMON_HW_H__