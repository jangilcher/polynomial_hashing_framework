#pragma once
/*
PATTERN TO ENCODE AEAD: NIBBLE  ENC-FAMILY      HASH        HASH-VARIANT
                        0       AES-256         MHP_NMH256     
                        1       RIJNDAEL256     POLY256
                        2       CHACHA20        MHP_NMH128
                        3       CHACHA16        POLY128
                        4       CHACHA12        MHP_NMH1163
                        5       RIJNDAEL256x    MHP_NMH2265
                        6                       POLY1163
                        7                       POLY2265
                        F                       NO_HASH
*/

#define RIJNDAEL_NO_HASH_X2X                  0x05F1
#define RIJNDAEL_NO_HASH_X3X                  0x05F2
#define RIJNDAEL_NO_HASH_X4X                  0x05F3
#define RIJNDAEL_NO_HASH_X6X                  0x05F4
#define RIJNDAEL_NO_HASH_X8X                  0x05F5

#define RIJNDAELx_POLY256_X2                  0x0511
#define RIJNDAELx_POLY256_X3                  0x0512
#define RIJNDAELx_POLY256_X4                  0x0513

#define RIJNDAELx_POLY128_X2                  0x0531
#define RIJNDAELx_POLY128_X4                  0x0532

#define RIJNDAELx_MHP_NMH256_X6                          0x0500
#define RIJNDAELx_MHP_NMH256_X2                          0x0501
#define RIJNDAELx_MHP_NMH256_X4                          0x0502

#define RIJNDAELx_MHP_NMH128_X6                          0x0520
#define RIJNDAELx_MHP_NMH128_X4                          0x0521

#define RIJNDAELx_MHP_NMH1163_X4                         0x0540
#define RIJNDAELx_MHP_NMH1163_X6                         0x0541

#define RIJNDAELx_MHP_NMH2265_X6                         0x0558
#define RIJNDAELx_MHP_NMH2265_X2                         0x0550
#define RIJNDAELx_MHP_NMH2265_X4                         0x0551

#ifndef __GNUC__
#pragma region RIJNDAELx
#endif
#if USED_AEAD == RIJNDAEL_NO_HASH_X2X
    #include "rijndael256/rijndael256_no_hash.h"
    #define AEAD rijndael256x2x_no_hash
#endif
#if USED_AEAD == RIJNDAEL_NO_HASH_X3X
    #include "rijndael256/rijndael256_no_hash.h"
    #define AEAD rijndael256x3x_no_hash
#endif
#if USED_AEAD == RIJNDAEL_NO_HASH_X4X
    #include "rijndael256/rijndael256_no_hash.h"
    #define AEAD rijndael256x4x_no_hash
#endif
#if USED_AEAD == RIJNDAEL_NO_HASH_X6X
    #include "rijndael256/rijndael256_no_hash.h"
    #define AEAD rijndael256x6x_no_hash
#endif
#if USED_AEAD == RIJNDAEL_NO_HASH_X8X
    #include "rijndael256/rijndael256_no_hash.h"
    #define AEAD rijndael256x8x_no_hash
#endif

#if USED_AEAD == RIJNDAELx_POLY256_X2
    #include "rijndael256/rijndael256x_poly256.h"
    #define AEAD rijndael256x_poly256_corex2
#endif
#if USED_AEAD == RIJNDAELx_POLY256_X3
    #include "rijndael256/rijndael256x_poly256.h"
    #define AEAD rijndael256x_poly256_corex3
#endif
#if USED_AEAD == RIJNDAELx_POLY256_X4
    #include "rijndael256/rijndael256x_poly256.h"
    #define AEAD rijndael256x_poly256_corex4
#endif

#if USED_AEAD == RIJNDAELx_POLY128_X2
    #include "rijndael256/rijndael256x_poly128.h"
    #define AEAD rijndael256x_poly128_corex2
#endif
#if USED_AEAD == RIJNDAELx_POLY128_X4
    #include "rijndael256/rijndael256x_poly128.h"
    #define AEAD rijndael256x_poly128_corex4
#endif

#if USED_AEAD == RIJNDAELx_MHP_NMH256_X2
    #include "rijndael256/rijndael256x_mhp_nmh256.h"
    #define AEAD rijndael256x_mhp_nmh256x2
#endif
#if USED_AEAD == RIJNDAELx_MHP_NMH256_X4
    #include "rijndael256/rijndael256x_mhp_nmh256.h"
    #define AEAD rijndael256x_mhp_nmh256x4
#endif
#if USED_AEAD == RIJNDAELx_MHP_NMH256_X6
    #include "rijndael256/rijndael256x_mhp_nmh256.h"
    #define AEAD rijndael256x_mhp_nmh256x6
#endif
#if USED_AEAD == RIJNDAELx_MHP_NMH128_X4
    #include "rijndael256/rijndael256x_mhp_nmh128.h"
    #define AEAD rijndael256x_mhp_nmh128x4
#endif
#if USED_AEAD == RIJNDAELx_MHP_NMH128_X6
    #include "rijndael256/rijndael256x_mhp_nmh128.h"
    #define AEAD rijndael256x_mhp_nmh128x6
#endif
#ifndef __GNUC__
#pragma endregion RIJNDAELx
#endif

#define BYTES_TO_VEC(arg) (((arg)/16) + (((arg)%16)!=0))

