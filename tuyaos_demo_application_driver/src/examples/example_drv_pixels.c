/**
 * @file example_drv_pixel.c
 * @author www.tuya.com
 * @brief example_drv_pixel module is used to 
 * @version 0.1
 * @date 2022-09-28
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include "tal_system.h"
#include "tal_log.h"
#include "tal_thread.h"

#if defined(ENABLE_SPI) && (ENABLE_SPI)
#include "tdd_pixel_ws2812.h"
#include "tdd_pixel_sm16703p.h"
#include "tdd_pixel_ws2814.h"
#include "tdd_pixel_sm16704pk.h"
#include "tdd_pixel_sm16714p.h"
#include "tdd_pixel_yx1903b.h"
#include "tdd_pixel_sk6812.h"
#endif

#include "tdl_pixel_dev_manage.h"
#include "tdl_pixel_color_manage.h"
#include "example_drv_pixels.h"
/***********************************************************
************************macro define************************
***********************************************************/
#define LED_PIXELS_TOTAL_NUM         50
#define LED_CHANGE_TIME              800 //ms
#define COLOR_RESOLUION              1000u

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC PIXEL_HANDLE_T sg_pixels_handle = NULL;
STATIC THREAD_HANDLE  sg_pixels_thrd = NULL;

/***********************************************************
*********************** const define ***********************
***********************************************************/
STATIC CONST PIXEL_COLOR_T cCOLOR_ARR[] = {
    { // red
        .warm = 0,
        .cold = 0,
        .red = COLOR_RESOLUION,
        .green = 0,
        .blue = 0,
    },
    { // green
        .warm = 0,
        .cold = 0,
        .red = 0,
        .green = COLOR_RESOLUION,
        .blue = 0,
    },
    { // blue
        .warm = 0,
        .cold = 0,
        .red = 0,
        .green = 0,
        .blue = COLOR_RESOLUION,
    },
    { // turn off
        .warm = 0,
        .cold = 0,
        .red = 0,
        .green = 0,
        .blue = 0,
    },
};

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC VOID __example_pixels_task(PVOID_T args)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T i = 0, color_num = CNTSOF(cCOLOR_ARR);

    while(1) {
        TUYA_CALL_ERR_GOTO(tdl_pixel_set_single_color(sg_pixels_handle, 0, LED_PIXELS_TOTAL_NUM, (PIXEL_COLOR_T *)&cCOLOR_ARR[i]), __ERROR);

        TUYA_CALL_ERR_GOTO(tdl_pixel_dev_refresh(sg_pixels_handle), __ERROR);

        i = (i+1)%color_num;
        tal_system_sleep(1000);
    }

__ERROR:
    TAL_PR_ERR("pixels demo error exit");
    tal_thread_delete(sg_pixels_thrd);
    sg_pixels_thrd = NULL;

    return;
}

/**
 * @brief    register hardware 
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET reg_pixels_hardware(CHAR_T *device_name)
{
    OPERATE_RET rt = OPRT_OK;

#if defined(ENABLE_SPI) && (ENABLE_SPI)
    PIXEL_DRIVER_CONFIG_T dev_init_cfg = {
        .port     = TUYA_SPI_NUM_0,
        .line_seq = RGB_ORDER,
    };

    //默认注册 ws2812 如果使用其他驱动，请开发者自行替换其他芯片驱动的接口 
    TUYA_CALL_ERR_RETURN(tdd_ws2812_driver_register(device_name, &dev_init_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_ws2812_opt_driver_register(device_name, &dev_init_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_ws2814_driver_register(device_name, &dev_init_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_sk6812_driver_register(device_name, &dev_init_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_sm16703p_driver_register(device_name, &dev_init_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_sm16704pk_driver_register(device_name, &dev_init_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_sm16714p_driver_register(device_name, &dev_init_cfg));

    // TUYA_CALL_ERR_RETURN(tdd_yx1903b_driver_register(device_name, &dev_init_cfg));

    return OPRT_OK;
#else 
    return OPRT_NOT_SUPPORTED;
#endif
}

/**
 * @brief    open driver
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET open_pixels_driver(CHAR_T *device_name)
{
    OPERATE_RET rt = OPRT_OK;

    /*find leds strip pixels device*/
    TUYA_CALL_ERR_RETURN(tdl_pixel_dev_find(device_name, &sg_pixels_handle));

    /*open leds strip pixels device*/
    PIXEL_DEV_CONFIG_T pixels_cfg = {
        .pixel_num        = LED_PIXELS_TOTAL_NUM,
        .pixel_resolution = COLOR_RESOLUION,
    };
    TUYA_CALL_ERR_RETURN(tdl_pixel_dev_open(sg_pixels_handle, &pixels_cfg));

    return OPRT_OK;
}

/**
 * @brief    the example of driver function
 *
 * @param[in] : the name of the driver
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET example_pixels_running(CHAR_T *device_name)
{
    /*create example task*/
    THREAD_CFG_T pixels_thrd_param = {2048, 4, "pixels_thread"};
    
    return tal_thread_create_and_start(&sg_pixels_thrd, NULL, NULL, __example_pixels_task, NULL, &pixels_thrd_param);
}