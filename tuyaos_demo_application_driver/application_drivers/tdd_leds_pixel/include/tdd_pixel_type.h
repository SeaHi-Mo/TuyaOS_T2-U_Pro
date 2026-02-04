/**
 * @file tdd_pixel_type.h
 * @brief tdd_pixel_type module is used to 
 * @version 0.1
 * @date 2022-09-05
 */

#ifndef __TDD_PIXEL_TYPE_H__
#define __TDD_PIXEL_TYPE_H__

#include "tkl_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef unsigned char RGB_ORDER_MODE_E;
#define RGB_ORDER 0x00
#define RBG_ORDER 0x01
#define GRB_ORDER 0x02
#define GBR_ORDER 0x03
#define BRG_ORDER 0x04
#define BGR_ORDER 0x05


typedef struct {
    TUYA_SPI_NUM_E port;
    RGB_ORDER_MODE_E line_seq;
} PIXEL_DRIVER_CONFIG_T;
/***********************************************************
********************function declaration********************
***********************************************************/


#ifdef __cplusplus
}
#endif

#endif /* __TDD_PIXEL_TYPE_H__ */
