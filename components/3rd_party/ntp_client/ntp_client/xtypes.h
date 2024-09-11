/**
 * @file xtypes.h
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * @author  ：Gaaagaa
 * @date    : 2019-11-29
 * @version : 1.0.0.0
 * @brief   : 公共的数据类型声明。
 */

#ifndef __XTYPES_H__
#define __XTYPES_H__

////////////////////////////////////////////////////////////////////////////////

#ifndef __X_TYPES_64__
    #if (defined(_WIN32) || defined(_WIN64))
        #ifdef _WIN64
            #define __X_TYPES_64__ 1
        #else // !_WIN64
            #define __X_TYPES_64__ 0
        #endif // _WIN64
    #elif (defined(__linux__) || defined(__unix__))
        #ifdef __x86_64__
            #define __X_TYPES_64__ 1
        #elif __i386__
            #define __X_TYPES_64__ 0
        #else // unknow
            // #error "unknow platform!"
            #define __X_TYPES_64__ 1
        #endif // __x86_64__
    #else // UNKNOW
        #define __X_TYPES_64__ 1
    #endif // PLATFORM
#endif // __X_TYPES_64__

////////////////////////////////////////////////////////////////////////////////

typedef char                   x_char_t;
typedef unsigned char          x_uchar_t;

typedef char                   x_int8_t;
typedef short                  x_int16_t;
typedef int                    x_int32_t;
typedef long long              x_int64_t;

typedef unsigned char          x_uint8_t;
typedef unsigned short         x_uint16_t;
typedef unsigned int           x_uint32_t;
typedef unsigned long long     x_uint64_t;

typedef long                   x_long_t;
typedef long long              x_llong_t;
typedef unsigned long          x_ulong_t;
typedef unsigned long long     x_ullong_t;

typedef float                  x_float_t;
typedef double                 x_lfloat_t;
typedef long double            x_llfloat_t;
typedef double                 x_double_t;
typedef long double            x_ldouble_t;

typedef void                   x_void_t;
typedef unsigned char          x_byte_t;
typedef unsigned int           x_bool_t;
typedef char *                 x_string_t;
typedef const char *           x_cstring_t;
typedef const char * const     x_ccstring_t;
typedef unsigned char *        x_bptr_t;
typedef void *                 x_handle_t;
typedef void *                 x_mptr_t;
typedef void *                 x_pvoid_t;

//======================================

#if __X_TYPES_64__
typedef x_llong_t              x_lsize_t;
typedef x_int64_t              x_ssize_t;
typedef x_uint64_t             x_size_t;
#else // !__X_TYPES_64__
typedef x_long_t               x_lsize_t;
typedef x_int32_t              x_ssize_t;
typedef x_uint32_t             x_size_t;
#endif // __X_TYPES_64__

#ifdef __linux__
typedef x_int32_t              x_sockfd_t;
#else // !__linux__
typedef x_ssize_t              x_sockfd_t;
#endif // __linux__

//======================================

// /**
//  * @brief
//  * @verbatim
//  * <pre>
//  *
//  * Variable name prefix
//  * @ xct_   : align at  8, for x_char_t, x_cchar_t, x_uchar_t, x_int8_t
//  * @ xwt_   : align at 16, for x_wchar_t, x_int16_t, x_uint16_t
//  * @ xit_   : align at 32, for x_int32_t
//  * @ xut_   : align at 32, for x_uint32_t, x_ulong_t
//  * @ xlt_   : align at 32, for x_long_t
//  * @ xlit_  : align at 64, for x_int64_t, x_llong_t
//  * @ xlut_  : align at 64, for x_uint64_t, x_ullong_t
//  * @ xbt_   : for x_bool_t
//  * @ xszt_  : for x_string_t, x_cstring_t, x_ccstring_t
//  * @ xwzt_  : for x_wstring_t, x_cwstring_t, x_ccwstring_t
//  * @ xht_   : for x_handle_t
//  * @ xpvt_  : for x_pvoid_t
//  * @ xlst_  : for x_lsize_t
//  * @ xst_   : for x_size_t
//  * @ xlpt_  : for x_lptr_t
//  * @ xfdt_  : for x_sockfd_t
//  * @ xft_   : for x_float_t
//  * @ xlft_  : for x_lfloat_t, x_double_t
//  *
//  * </pre>
//  * @endverbatim
//  */

////////////////////////////////////////////////////////////////////////////////

#define X_FALSE           0
#define X_TRUE            1
#define X_NULL            0
#define X_ERR_OK          0
#define X_ERR_UNKNOW      (-1)

#define X_INVALID_SOCKFD  ((x_sockfd_t)~0)

#define TEXT_LEN_16       16
#define TEXT_LEN_24       24
#define TEXT_LEN_32       32
#define TEXT_LEN_48       48
#define TEXT_LEN_64       64
#define TEXT_LEN_96       96
#define TEXT_LEN_128      128
#define TEXT_LEN_256      256
#define TEXT_LEN_512      512
#define TEXT_LEN_768      768
#define TEXT_LEN_1K       1024
#define TEXT_LEN_2K       2048
#define TEXT_LEN_4K       4096
#define TEXT_LEN_PATH     260

////////////////////////////////////////////////////////////////////////////////

#endif // __XTYPES_H__
