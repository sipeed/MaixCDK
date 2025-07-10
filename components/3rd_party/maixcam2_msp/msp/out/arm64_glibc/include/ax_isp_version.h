/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __AX_ISP_VERSION_H__
#define __AX_ISP_VERSION_H__

#define CHIP_NAME    "AX620E"

#define SDK_VER_PRIX "_ISP_V"

#define ISP_VERSION_MAJOR   4
#define ISP_VERSION_MINOR   0
#define ISP_VERSION_MINOR2  39

#define __MAKE_VERSION(a,x,y) #a"."#x"."#y
#define MAKE_VERSION(a,x,y)     __MAKE_VERSION(a,x,y)
#define ISP_VERSION  CHIP_NAME SDK_VER_PRIX MAKE_VERSION(ISP_VERSION_MAJOR,ISP_VERSION_MINOR,ISP_VERSION_MINOR2)


#define AX_ISP_VERSION_MAX_SIZE         (64)
#define AX_ISP_BUILD_TIME_MAX_SIZE      (32)

#endif //__AX_ISP_VERSION_H__
