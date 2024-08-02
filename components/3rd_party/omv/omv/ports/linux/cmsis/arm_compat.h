#ifndef __ARM_COMPAT__H
#define __ARM_COMPAT__H
#ifdef __cplusplus
extern "C"
{
#endif

// typedef __uint8_t uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
// typedef __uint64_t uint64_t;
#ifndef   __STATIC_FORCEINLINE
  #define __STATIC_FORCEINLINE                   __attribute__((always_inline)) static inline
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__



#else //__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__


// Reverse the bit order in a 32-bit word.
__STATIC_FORCEINLINE uint32_t __RBIT(
  uint32_t i )
{
  i = ( ( i & 0x55555555 ) << 1 ) | ( ( i >> 1 ) & 0x55555555 );
  i = ( ( i & 0x33333333 ) << 2 ) | ( ( i >> 2 ) & 0x33333333 );
  i = ( ( i & 0x0f0f0f0f ) << 4 ) | ( ( i >> 4 ) & 0x0f0f0f0f );
  i = ( i << 24 ) | ( ( i & 0xff00 ) << 8 ) //
    | ( ( i >> 8 ) & 0xff00 ) | ( i >> 24 );
  return i;
}

// Reverse byte order in each halfword independently
// converts 16-bit big-endian data into little-endian data
// or 16-bit little-endian data into big-endian data
__STATIC_FORCEINLINE short __REV16(
  short s )
{
  return ( s << 8 ) | ( s >> 8 );
}

// Reverse byte order in a word
// converts 32-bit big-endian data into little-endian data
// or 32-bit little-endian data into big-endian data.
__STATIC_FORCEINLINE uint32_t __REV32(
  uint32_t i )
{
  return ( i & 0x000000FFU ) << 24 | ( i & 0x0000FF00U ) << 8
    | ( i & 0x00FF0000U ) >> 8 | ( i & 0xFF000000U ) >> 24;
}

__STATIC_FORCEINLINE uint32_t __SSUB16(uint32_t op1, uint32_t op2)
{
  return ((op1 & 0xFFFF0000) - (op2 & 0xFFFF0000)) | ((op1 - op2) & 0xFFFF);
}

__STATIC_FORCEINLINE uint32_t __UXTB_RORn(uint32_t op1, uint32_t rotate)
{
  return (op1 >> rotate) & 0xFF;
}

#define __CLZ             (uint8_t)__builtin_clz

#define __PKHBT(ARG1,ARG2,ARG3)          ( ((((uint32_t)(ARG1))          ) & 0x0000FFFFUL) |  \
                                           ((((uint32_t)(ARG2)) << (ARG3)) & 0xFFFF0000UL)  )

#define __PKHTB(ARG1,ARG2,ARG3)          ( ((((uint32_t)(ARG1))          ) & 0xFFFF0000UL) |  \
                                           ((((uint32_t)(ARG2)) >> (ARG3)) & 0x0000FFFFUL)  )

__STATIC_FORCEINLINE uint32_t __SMLAD (uint32_t op1, uint32_t op2, uint32_t op3)
{
  uint32_t result;
  uint16_t *op1_s = (uint16_t *) &op1, *op2_s = (uint16_t *) &op2;

  result = op1_s[0] * op2_s[0];
  result += op1_s[1] * op2_s[1];
  result += op3;

  return result;
}

__STATIC_FORCEINLINE uint32_t __SMUAD  (uint32_t op1, uint32_t op2)
{
  uint32_t result;
  uint16_t *op1_s = (uint16_t *) &op1, *op2_s = (uint16_t *) &op2;

  result = op1_s[0] * op2_s[0] + op1_s[1] * op2_s[1];

  return result;
}

__STATIC_FORCEINLINE uint32_t __QADD16(uint32_t op1, uint32_t op2)
{
  uint32_t result;
  uint16_t *op1_s = (uint16_t *) &op1, *op2_s = (uint16_t *) &op2, *result_s = (uint16_t *) &result;

  result_s[0] = op1_s[0] + op2_s[0];
  result_s[1] = op1_s[1] + op2_s[1];
  return(result);
}

__STATIC_FORCEINLINE uint32_t __SMLADX (uint32_t op1, uint32_t op2, uint32_t op3)
{
  uint32_t result;
  uint16_t *op1_s = (uint16_t *) &op1, *op2_s = (uint16_t *) &op2;

  result = op1_s[0] * op2_s[1];
  result += op1_s[1] * op2_s[0];
  result += op3;

  return result;
}

#define __USAT_ASR(ARG1,ARG2,ARG3) \
({                          \
  uint32_t __RES, __ARG1 = (ARG1), __ARG2 = 1<<ARG2; \
  __ARG1 = __ARG1>>ARG3; \
  if(__ARG1 >= __ARG2) __RES = __ARG2; \
  else __RES = __ARG1;  \
  __RES; \
 })

#define arm_sin_f32(x) sinf(x)
#define arm_cos_f32(x) cosf(x)

#define __USAT(val1, val2) \
({\
    __typeof__ (val1) _val1 = val1;\
    __typeof__ (val2) _val2 = val2;\
    ((uint32_t)((0xffffffff >> (32 - _val2)) & _val1));\
})
#define __USAT16(val1, val2) \
({\
    __typeof__ (val1) _val1 = val1;\
    __typeof__ (val2) _val2 = val2;\
    ((_val1 & ((0xffff >> (16 - _val2)) << 16)) | (_val1 & (0xffff >> (16 - _val2))));\
})
#endif //__BYTE_ORDER__ == __ORDER_LITTER_ENDIAN__


#pragma GCC diagnostic pop  // -Wuninitialized

#ifdef __cplusplus
}
#endif

#endif //__ARM_COMPAT__H



#ifdef _CC_ARM_asdxasxsaadsadsaxasadasdsadsads
//备份宏定义
#define __SMLAD(x, y, sum) \
({\
    __typeof__ (x) __x = x;\
    __typeof__ (y) __y = y;\
    __typeof__ (sum) __sum = sum;\
    ((uint32_t)(((((int32_t)__x << 16) >> 16) * (((int32_t)__y << 16) >> 16)) + ((((int32_t)__x) >> 16) * (((int32_t)__y) >> 16)) + ( ((int32_t)__sum))));\
})

#define __SMUAD(val1, val2) \
({\
    __typeof__ (val1) _val1 = val1;\
    __typeof__ (val2) _val2 = val2;\
    ((uint32_t)(((((int32_t)_val1 << 16) >> 16) * (((int32_t)_val2 << 16) >> 16)) + ((((int32_t)_val1) >> 16) * (((int32_t)_val2) >> 16))));\
})


#define __QADD16(val1, val2) \
({\
    __typeof__ (val1) _val1 = val1;\
    __typeof__ (val2) _val2 = val2;\
    ((uint32_t)((((((int32_t)_val1 << 16) >> 16) + (((int32_t)_val2 << 16) >> 16))) | (((((int32_t)_val1) >> 16) + (((int32_t)_val2) >> 16)) << 16)));\
})

#define __USAT(val1, val2) \
({\
    __typeof__ (val1) _val1 = val1;\
    __typeof__ (val2) _val2 = val2;\
    ((uint32_t)((0xffffffff >> (32 - _val2)) & _val1));\
})

#define __USAT16(val1, val2) \
({\
    __typeof__ (val1) _val1 = val1;\
    __typeof__ (val2) _val2 = val2;\
    ((_val1 & ((0xffff >> (16 - _val2)) << 16)) | (_val1 & (0xffff >> (16 - _val2))));\
})

#define __SSUB16(val1, val2) \
({\
    __typeof__ (val1) _val1 = val1;\
    __typeof__ (val2) _val2 = val2;\
    ((((_val1 >> 16) - (_val2 >> 16)) << 16) | ((_val1 & 0xffff) - (_val2 & 0xffff)));\
})

#define __REV16(_x) __builtin_bswap16(_x)


#define __CLZ(val1) \
({\
    __typeof__ (val1) _val1 = val1;\
    uint32_t tmp_0 = 0, tmp_1 = 0x80000000, tmp_2 = 0;\
    if(_val1 == 0){tmp_2 = 32;}else{for(tmp_0 = 0; tmp_0 < 32; tmp_0 ++){if(_val1 & tmp_1){break;}else{tmp_2 ++;tmp_1 = tmp_1 >> 1;}}}\
    tmp_2;\
})
#endif //_CC_ARM_asdxasxsaadsadsaxasadasdsadsadsaxsa
