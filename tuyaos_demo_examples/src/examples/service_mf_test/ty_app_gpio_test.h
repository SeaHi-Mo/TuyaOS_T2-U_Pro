
/**
 * @file ty_app_gpio_test.h
 * @brief ty_app_gpio_test module is used to 
 * @version 0.1
 * @date 2022-10-31
 */

#ifndef __TY_APP_GPIO_TEST_H__
#define __TY_APP_GPIO_TEST_H__

#include "tuya_iot_config.h"
#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define gpio_test_enabe(enable)  ty_app_gpio_test_enabe(enable)

#define gpio_test_all(in,out)    ty_app_gpio_test_all(in,out)

/***********************************************************
***********************typedef define***********************
***********************************************************/
/**
 * @brief Definition group of control
 */
typedef struct {
    INT_T   ionum;
    TUYA_GPIO_NUM_E iopin[8];
} CTRL_GROUP;

/**
 * @brief Definition gpio test table
 *
 */
typedef struct {
    INT_T group_num;
    CTRL_GROUP group[20];
} GPIO_TEST_TABLE;

/***********************************************************
********************function declaration********************
***********************************************************/
/**
 * @brief enable gpio test
 *
 * @param[in] enable Flag to enable
 *
 */
VOID ty_app_gpio_test_enabe(BOOL_T enable);

/**
 * @brief gpio test function
 *
 * @param[in] in Test content in
 * @param[out] out Test content out
 *
 * @return TRUE/FLASE
 */
BOOL_T ty_app_gpio_test_all(IN CONST CHAR_T *in, OUT CHAR_T *out);

#ifdef __cplusplus
}
#endif

#endif /* __TY_APP_GPIO_TEST_H__ */





