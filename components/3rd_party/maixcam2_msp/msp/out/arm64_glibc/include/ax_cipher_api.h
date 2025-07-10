/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __AX_CIPHER_H__
#define __AX_CIPHER_H__
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/** Cipher algorithm */
typedef enum {
    AX_CIPHER_ALGO_HASH_SHA1    = 0,          // SHA-1
    AX_CIPHER_ALGO_HASH_SHA224  = 1,          // SHA-224
    AX_CIPHER_ALGO_HASH_SHA256  = 2,          // SHA-256
    AX_CIPHER_ALGO_HASH_SHA384  = 3,          // SHA-384
    AX_CIPHER_ALGO_HASH_SHA512  = 4,          // SHA-512
    AX_CIPHER_ALGO_MAC_HMAC_SHA1 = 5,         // HMAC-SHA-1
    AX_CIPHER_ALGO_MAC_HMAC_SHA224 = 6,       // HMAC-SHA-224
    AX_CIPHER_ALGO_MAC_HMAC_SHA256 = 7,       // HMAC-SHA-256
    AX_CIPHER_ALGO_MAC_HMAC_SHA384 = 8,       // HMAC-SHA-384
    AX_CIPHER_ALGO_MAC_HMAC_SHA512 = 9,       // HMAC-SHA-512
    AX_CIPHER_ALGO_MAC_AES_CMAC = 10,         // AES-CMAC
    AX_CIPHER_ALGO_MAC_AES_CBC_MAC = 11,      // AES-CBC-MAC
    AX_CIPHER_ALGO_CIPHER_AES = 12,           // AES
    AX_CIPHER_ALGO_CIPHER_DES = 13,           // DES
    AX_CIPHER_ALG_INVALID = 0xffffffff,
} AX_CIPHER_ALGO_E;
typedef enum {
    // (Block)Cipher modes
    AX_CIPHER_MODE_CIPHER_ECB = 0,        // ECB
    AX_CIPHER_MODE_CIPHER_CBC,            // CBC
    AX_CIPHER_MODE_CIPHER_CTR,            // CTR
    AX_CIPHER_MODE_CIPHER_ICM,            // ICM
    AX_CIPHER_MODE_CIPHER_F8,             // F8
    AX_CIPHER_MODE_CIPHER_CCM,            // CCM
    AX_CIPHER_MODE_CIPHER_XTS,            // XTS
    AX_CIPHER_MODE_CIPHER_GCM,            // GCM
    AX_CIPHER_MODE_CIPHER_MAX,            // must be last
} AX_CIPHER_MODE_E;

typedef enum AX_CIPHER_RSA_SIGN_SCHEME_E {
    AX_CIPHER_RSA_SIGN_RSASSA_PKCS1_V15_SHA1 = 0x0,
    AX_CIPHER_RSA_SIGN_RSASSA_PKCS1_V15_SHA224,
    AX_CIPHER_RSA_SIGN_RSASSA_PKCS1_V15_SHA256,
    AX_CIPHER_RSA_SIGN_RSASSA_PKCS1_V15_SHA384,
    AX_CIPHER_RSA_SIGN_RSASSA_PKCS1_V15_SHA512,
    AX_CIPHER_RSA_SIGN_RSASSA_PKCS1_PSS_SHA1,
    AX_CIPHER_RSA_SIGN_RSASSA_PKCS1_PSS_SHA224,
    AX_CIPHER_RSA_SIGN_RSASSA_PKCS1_PSS_SHA256,
    AX_CIPHER_RSA_SIGN_RSASSA_PKCS1_PSS_SHA384,
    AX_CIPHER_RSA_SIGN_RSASSA_PKCS1_PSS_SHA512,
    AX_CIPHER_RSA_SIGN_SCHEME_INVALID  = 0xffffffff,
} AX_CIPHER_RSA_SIGN_SCHEME_E;

typedef struct {
    AX_U8 *hmacKey;
    AX_U32 hmackeyLen;
    AX_CIPHER_ALGO_E hashType;
} AX_CIPHER_HASH_CTL_T;

typedef struct {
    AX_U32 hashBits;
    AX_U32 modulusBits;
    AX_U8 *modulusData;
    AX_U32 privateExponentBytes;
    AX_U8 *exponentData;
    AX_CIPHER_RSA_SIGN_SCHEME_E enScheme;
} AX_CIPHER_RSA_PRIVATE_KEY;

typedef struct {
    AX_U32 hashBits;
    AX_U32 modulusBits;
    AX_U8 *modulusData;
    AX_U32 publicExponentBytes;
    AX_U8 *exponentData;
    AX_CIPHER_RSA_SIGN_SCHEME_E enScheme;
} AX_CIPHER_RSA_PUBLIC_KEY;

typedef struct {
    AX_U8 *data;
    AX_U32 len;             // Data size in bytes
} AX_CIPHER_SIG_DATA_T;

typedef enum {
    AX_CIPHER_RSA_ENC_SCHEME_NO_PADDING,
    AX_CIPHER_RSA_ENC_SCHEME_PKCS1_V1_5,
} AX_CIPHER_RSA_ENC_SCHEME_E;

typedef struct {
    AX_CIPHER_RSA_ENC_SCHEME_E enScheme;
    AX_CIPHER_RSA_PUBLIC_KEY pubKey;
} AX_CIPHER_RSA_PUB_ENC_T;

typedef struct {
    AX_CIPHER_RSA_ENC_SCHEME_E enScheme;
    AX_CIPHER_RSA_PRIVATE_KEY priKey;
} AX_CIPHER_RSA_PRI_ENC_T;

#ifndef AX_SUCCESS
#define AX_SUCCESS                          0
#endif

typedef enum {
    AX_ERR_CIPHER_ILLEGAL_PARAM = 0x8002000A,         /** Invalid parameter */
    AX_ERR_CIPHER_ACCESS_ERROR = 0x80020080,              /** Access error */
    AX_ERR_CIPHER_BUF_FULL = 0x80020021,               /** Full/Overflow error */
    AX_ERR_CIPHER_INVALID_ALGORITHM = 0x80020081,       /** Invalid algorithm code */
    AX_ERR_CIPHER_NOMEM = 0x80020018,               /** No memory */
    AX_ERR_CIPHER_OPERATION = 0x80020082,        /** Operation failed */
    AX_ERR_CIPHER_INTERNAL = 0x80020083,          /** Internal error */
    AX_ERR_CIPHER_OPEN = 0x80020084,          	/** load dev error */
} AX_CIPHER_STS;


typedef AX_U64                  AX_CIPHER_HANDLE;
typedef struct {
    AX_CIPHER_ALGO_E alg;        /**< Cipher algorithm */
    AX_CIPHER_MODE_E workMode;   /**< Operating mode */
    AX_U8 *pKey;               /**< Key input */
    AX_U32 keySize;                /**< Key size */
    AX_U8 *pIV;                /**< Initialization vector (IV) */
} AX_CIPHER_CTRL_T;

AX_S32 AX_CIPHER_Init(AX_VOID);
AX_S32 AX_CIPHER_DeInit(AX_VOID);
AX_S32 AX_CIPHER_CreateHandle(AX_CIPHER_HANDLE *phCipher, const AX_CIPHER_CTRL_T *pstCipherCtrl);
AX_S32 AX_CIPHER_Encrypt(AX_CIPHER_HANDLE pCipher, AX_U8 *szSrcAddr, AX_U8 *szDestAddr, AX_U32 byteLength);
AX_S32 AX_CIPHER_Decrypt(AX_CIPHER_HANDLE pCipher, AX_U8 *szSrcAddr, AX_U8 *szDestAddr, AX_U32 byteLength);
AX_S32 AX_CIPHER_EncryptPhy(AX_CIPHER_HANDLE pCipher, AX_U64 szSrcAddr, AX_U64 szDestAddr, AX_U32 byteLength);
AX_S32 AX_CIPHER_DecryptPhy(AX_CIPHER_HANDLE pCipher, AX_U64 szSrcAddr, AX_U64 szDestAddr, AX_U32 byteLength);
AX_S32 AX_CIPHER_DestroyHandle(AX_CIPHER_HANDLE pCipher);
AX_S32 AX_CIPHER_RsaVerify(AX_CIPHER_RSA_PUBLIC_KEY *key, AX_U8 *msg, AX_U32 msgBytes, AX_CIPHER_SIG_DATA_T *sig);
AX_S32 AX_CIPHER_RsaSign(AX_CIPHER_RSA_PRIVATE_KEY *key, AX_U8 *msg, AX_U32 msgBytes, AX_CIPHER_SIG_DATA_T *sig);
AX_U32 AX_CIPHER_RsaPublicEncrypt(AX_CIPHER_RSA_PUB_ENC_T *pRsaEnc, AX_U8 *pInput, AX_U32 inLen, AX_U8 *pOutput, AX_U32 *pOutLen);
AX_U32 AX_CIPHER_RsaPublicDecrypt(AX_CIPHER_RSA_PUB_ENC_T *pRsaDec, AX_U8 *pInput, AX_U32 inLen, AX_U8 *pOutput, AX_U32 *pOutLen);
AX_U32 AX_CIPHER_RsaPrivateEncrypt(AX_CIPHER_RSA_PRI_ENC_T *pRsaEnc, AX_U8 *pInput, AX_U32 inLen, AX_U8 *pOutput, AX_U32 *pOutLen);
AX_U32 AX_CIPHER_RsaPrivateDecrypt(AX_CIPHER_RSA_PRI_ENC_T *pRsaDec, AX_U8 *pInput, AX_U32 inLen, AX_U8 *pOutput, AX_U32 *pOutLen);
AX_S32 AX_CIPHER_HashInit(AX_CIPHER_HASH_CTL_T *pstHashCtl, AX_CIPHER_HANDLE *pHashHandle);
AX_S32 AX_CIPHER_HashUpdate(AX_CIPHER_HANDLE handle, AX_U8 *inputData, AX_U32 inPutLen);
AX_S32 AX_CIPHER_HashFinal(AX_CIPHER_HANDLE handle, AX_U8 *inputData, AX_U32 inPutLen, AX_U8 *outPutHash);
AX_S32 AX_CIPHER_HashUpdatePhy(AX_CIPHER_HANDLE handle, AX_U64 inputPhy, AX_U32 inputLen);
AX_S32 AX_CIPHER_HashFinalPhy(AX_CIPHER_HANDLE handle, AX_U64 inputPhy, AX_U32 inputLen, AX_U8 *outPutHash);
AX_U32 AX_CIPHER_GetRandomNumber(AX_U32 *pRandomNumber, AX_U32 size);

//
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AX_CIPHER_H__ */
