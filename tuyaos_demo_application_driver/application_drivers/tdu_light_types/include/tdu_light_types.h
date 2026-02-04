/**
 * @file tdu_light_types.h
 * @author www.tuya.com
 * @brief tdl_light_types module is used to provide common
 *        types of lighting products
 * @version 0.1
 * @date 2022-08-22
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TDU_LIGHT_TYPES_H__
#define __TDU_LIGHT_TYPES_H__

#include "tuya_cloud_types.h"
#include "tal_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define LIG_THREE_MAX(x,y,z)                      (x)>(y)?((x)>(z)?(x):(z)):((y)>(z)?(y):(z))

#define LIG_PERCENT_TO_INT(percent, base)         (((FLOAT_T)(percent)/100.0)*(base))  //将百分比变成具体数值
#define LIG_RATE_TO_TIME(rate, min_time)          ((min_time)*100/(rate))     //将速度占比转化成具体时间

#define LIGHT_COLOR_CHANNEL_MAX                   5u

/**
 * @brief light product type
 */
#define LIGHT_PRODUCT_DIMMER                     0 //单像素全彩
#define LIGHT_PRODUCT_PIXELS                     1 //多像素幻彩


/**
 * @brief color channel idx
 */
#define COLOR_CH_IDX_RED                          0
#define COLOR_CH_IDX_GREEN                        1
#define COLOR_CH_IDX_BLUE                         2
#define COLOR_CH_IDX_COLD                         3
#define COLOR_CH_IDX_WARM                         4

/**
 * @brief light device type - bit code
 */
#define LIGHT_R_BIT             0x01
#define LIGHT_G_BIT             0x02
#define LIGHT_B_BIT             0x04
#define LIGHT_C_BIT             0x08
#define LIGHT_W_BIT             0x10

#define LIGHT_CW_BIT           (LIGHT_C_BIT | LIGHT_W_BIT)
#define LIGHT_RGB_BIT          (LIGHT_R_BIT | LIGHT_G_BIT | LIGHT_B_BIT)
#define LIGHT_RGBCW_BIT        (LIGHT_R_BIT | LIGHT_G_BIT | LIGHT_B_BIT | LIGHT_C_BIT | LIGHT_W_BIT)

/**
 * @brief light device name length
 */
#define LIGHT_NAME_MAX_LEN      16u

/**
 * @brief light color the value hue
 */
#define LIGHT_COLOR_HUE_MAX        360
#define LIGHT_COLOR_HUE_MIN        0
#define LIGHT_COLOR_HUE_RED        0
#define LIGHT_COLOR_HUE_YELLOW     60
#define LIGHT_COLOR_HUE_GREEN      120
#define LIGHT_COLOR_HUE_CYAN       180
#define LIGHT_COLOR_HUE_BLUE       240
#define LIGHT_COLOR_HUE_PURPLE     300
#define LIGHT_COLOR_HUE_WHITE      0

/**
 * @brief parameter checking
 */
#define TY_APP_PARAM_CHECK(condition)                                                                                  \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            TAL_PR_ERR("Pre check(" #condition "):in line %d", __LINE__);                                              \
            return OPRT_INVALID_PARM;                                                                                  \
        }                                                                                                              \
    } while (0)

/**
 * @brief parameter checking without return value
 */
#define TY_APP_PARAM_CHECK_NO_RET(condition)                                                                           \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            TAL_PR_ERR("Pre check(" #condition "):in line %d", __LINE__);                                              \
            return ;                                                                                                   \
        }                                                                                                              \
    } while (0)


#define LIGHT_DVE_TYPE(r,g,b,c,w) (r | (g<<1) | (b<<2) | (c<<3) | (w<<4))

/***********************************************************
***********************typedef define***********************
***********************************************************/
/**
 * @brief light device type
 */
typedef BYTE_T LIGHT_DEV_TP_E;
#define LIGHT_DEV_TP_C          (LIGHT_C_BIT)
#define LIGHT_DEV_TP_CW         (LIGHT_C_BIT | LIGHT_W_BIT)
#define LIGHT_DEV_TP_RGB        (LIGHT_R_BIT | LIGHT_G_BIT | LIGHT_B_BIT)
#define LIGHT_DEV_TP_RGBC       (LIGHT_R_BIT | LIGHT_G_BIT | LIGHT_B_BIT | LIGHT_C_BIT)
#define LIGHT_DEV_TP_RGBW       (LIGHT_R_BIT | LIGHT_G_BIT | LIGHT_B_BIT | LIGHT_W_BIT)
#define LIGHT_DEV_TP_RGBCW      (LIGHT_R_BIT | LIGHT_G_BIT | LIGHT_B_BIT | LIGHT_C_BIT | LIGHT_W_BIT)

typedef BYTE_T LIGHT_DRV_COLOR_CH_E;
#define LIGHT_DRV_COLOR_CH_C          (LIGHT_C_BIT)
#define LIGHT_DRV_COLOR_CH_CW         (LIGHT_C_BIT | LIGHT_W_BIT)
#define LIGHT_DRV_COLOR_CH_RGB        (LIGHT_R_BIT | LIGHT_G_BIT | LIGHT_B_BIT)
#define LIGHT_DRV_COLOR_CH_RGBC       (LIGHT_R_BIT | LIGHT_G_BIT | LIGHT_B_BIT | LIGHT_C_BIT)
#define LIGHT_DRV_COLOR_CH_RGBW       (LIGHT_R_BIT | LIGHT_G_BIT | LIGHT_B_BIT | LIGHT_W_BIT)
#define LIGHT_DRV_COLOR_CH_RGBCW      (LIGHT_R_BIT | LIGHT_G_BIT | LIGHT_B_BIT | LIGHT_C_BIT | LIGHT_W_BIT)
/**
 * @brief light mode
 */
typedef BYTE_T LIGHT_MODE_E;
#define LIGHT_MODE_WHITE           0x00
#define LIGHT_MODE_COLOR           0x01
#define LIGHT_MODE_SCENE           0x02
#define LIGHT_MODE_MUSIC           0x03
#define LIGHT_MODE_NUM             0x04
#define LIGHT_MODE_INVALID         LIGHT_MODE_NUM

typedef BYTE_T LIGHT_PAINT_MODE_E;
#define LIGHT_PAINT_MODE_WHITE     LIGHT_MODE_WHITE    
#define LIGHT_PAINT_MODE_COLOR     LIGHT_MODE_COLOR 
#define LIGHT_PAINT_MODE_INVALID   LIGHT_MODE_INVALID

typedef BYTE_T LIGHT_COLOR_TYPE_E;
#define LIGHT_COLOR_RGBCW          0x00
#define LIGHT_COLOR_HSVBT          0x01
#define LIGHT_COLOR_AXIS           0x02

typedef UINT_T LIGHT_COLOR_E;
#define LIGHT_COLOR_C              0x00
#define LIGHT_COLOR_CW             0x01
#define LIGHT_COLOR_R              0x02
#define LIGHT_COLOR_G              0x03
#define LIGHT_COLOR_B              0x04
#define LIGHT_COLOR_RGB            0x05
#define LIGHT_COLOR_NUM_MAX        0x06


/**
 * @brief light mixing type
 */
typedef BYTE_T LIGHT_MIX_TP_E;
#define LIGHT_MIX_TP_NONE       0x00    // no light mixing
#define LIGHT_MIX_TP_TEMPER     0x01    

/**
 * @brief hsv data
 */
typedef struct {
    USHORT_T hue;
    USHORT_T sat;
    USHORT_T val;
} COLOR_HSV_T, LIGHT_HSV_T;

/**
 * @brief rgb data
 */
typedef struct {
    USHORT_T red;
    USHORT_T green;
    USHORT_T blue;
} COLOR_RGB_T;

/**
 * @brief bt data
 */
typedef struct {
    USHORT_T bright;
    USHORT_T temper;
} WHITE_BT_T;

/**
 * @brief cw data
 */
typedef struct {
    USHORT_T cold;
    USHORT_T warm;
} WHITE_CW_T;

/**
 * @brief hsvbt data
 */
typedef struct {
    USHORT_T hue;
    USHORT_T sat;
    USHORT_T val;
    USHORT_T bright;
    USHORT_T temper;
} LIGHT_HSVBT_T;

typedef struct {
    LIGHT_PAINT_MODE_E        mode;
    COLOR_HSV_T               hsv;
    WHITE_BT_T                bt;
}LIGHT_PAINT_T;

/**
 * @brief rgbcw data
 */
#pragma pack(2)
typedef struct {
    USHORT_T red;
    USHORT_T green;
    USHORT_T blue;
    USHORT_T cold;
    USHORT_T warm;
}LIGHT_RGBCW_T, PIXEL_COLOR_T;

#pragma pack()
typedef union {
    LIGHT_RGBCW_T s;
    USHORT_T      array[LIGHT_COLOR_CHANNEL_MAX];
}LIGHT_RGBCW_U;

typedef struct {
    BOOL_T   enable;
    UCHAR_T  pin;
    BOOL_T   active_lv;
}LIGHT_SWITCH_PIN_CFG_T;

/***********************************************************
********************function declaration********************
***********************************************************/



#ifdef __cplusplus
}
#endif

#endif /* __TDL_LIGHT_TYPES_H__ */
