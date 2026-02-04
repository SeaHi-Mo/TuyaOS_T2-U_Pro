/**
 * @file example_driver_dvp.c
 * @brief example_driver_dvp module is used to 
 * @version 0.1
 * @date 2025-05-30
 */

#include "tuya_cloud_types.h"

#include "tal_time_service.h"
#include "tal_log.h"
#include "tal_mutex.h"

#include "tkl_video_in.h"
#include "tkl_video_enc.h"
#include "tkl_fs.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define VIDEO_FILE_DIR       "/sdcard"

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
#define CAPTURED_FRAME_PATH_LEN 128
static char captured_frame_path[CAPTURED_FRAME_PATH_LEN] = {0};

static MUTEX_HANDLE sg_dvp_mutex = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/

static OPERATE_RET app_fs_init(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    rt = tkl_fs_mount(VIDEO_FILE_DIR, DEV_SDCARD);
    if (rt != OPRT_OK) {
        TAL_PR_ERR("mount sd card failed, please retry after format ");
        return rt;
    }
    TAL_PR_DEBUG("mount sd card success ");

    return rt;
}

STATIC OPERATE_RET app_save_file(CHAR_T *file_path, VOID_T *data, UINT32_T data_len)
{
    OPERATE_RET rt = OPRT_OK;

    if (file_path == NULL) {
        TAL_PR_ERR("file_path is NULL");
        return OPRT_INVALID_PARM;
    }

    // Check if the file already exists
    BOOL_T is_exist = FALSE;
    rt = tkl_fs_is_exist(file_path, &is_exist);
    if (rt != OPRT_OK) {
        TAL_PR_ERR("tkl_fs_is_exist failed, rt = %d", rt);
        return rt;
    }
    if (is_exist) {
        tkl_fs_remove(file_path);
        TAL_PR_DEBUG("remove file %s", file_path);
    }

    // Create the file
    TUYA_FILE file_hdl = tkl_fopen(file_path, "w");
    if (file_hdl == NULL) {
        TAL_PR_ERR("Failed to create file %s", file_path);
        return OPRT_FILE_OPEN_FAILED;
    }
    TAL_PR_NOTICE("File %s created successfully", file_path);

    // Write data to the file
    INT_T write_len = tkl_fwrite(data, data_len, file_hdl);
    if (write_len != data_len) {
        TAL_PR_ERR("Failed to write data to file %s, expected %d bytes, wrote %d bytes", 
                   file_path, data_len, write_len);
        tkl_fclose(file_hdl);
        return OPRT_COM_ERROR;
    }
    TAL_PR_NOTICE("Data written to file %s successfully, length: %d bytes", file_path, write_len);
    // Close the file
    rt = tkl_fclose(file_hdl);
    if (rt != OPRT_OK) {
        TAL_PR_ERR("Failed to close file %s, rt = %d", file_path, rt);
        return rt;
    }

    return rt;
}

static INT_T __h264_cb(TKL_VENC_FRAME_T *pframe)
{
    OPERATE_RET rt = OPRT_OK;

    if (pframe->pbuf == NULL || pframe->buf_size == 0) {
        return 0;
    }

    if (pframe->frametype != TKL_VIDEO_I_FRAME) {
        return 0;
    }

    if (sg_dvp_mutex == NULL) {
        TAL_PR_ERR("DVP mutex is NULL");
        return 0;
    }

    tal_mutex_lock(sg_dvp_mutex);

    memset(captured_frame_path, 0, CAPTURED_FRAME_PATH_LEN);

    POSIX_TM_S local_tm;
    memset((UCHAR_T *)&local_tm, 0x00, SIZEOF(POSIX_TM_S));
    rt = tal_time_get_local_time_custom(0, &local_tm);
    if (rt != OPRT_OK) {
        TAL_PR_ERR("Failed to get local time, rt = %d", rt);
        return 0;
    }

    snprintf(captured_frame_path, CAPTURED_FRAME_PATH_LEN, "%s/%02d_%02d_%02d", VIDEO_FILE_DIR, local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
    TAL_PR_DEBUG("File name: %s", captured_frame_path);
    // Save the frame to a file
    rt = app_save_file(captured_frame_path, pframe->pbuf, pframe->buf_size);
    if (rt != OPRT_OK) {
        TAL_PR_ERR("Failed to save file, rt = %d", rt);
        return 0;
    }
    TAL_PR_DEBUG("File saved successfully: %s", captured_frame_path);

    tal_mutex_unlock(sg_dvp_mutex);

    return 0;
}

static VOID example_dvp_init(VOID)
{
    // Initialize the DVP driver
    TKL_VI_CONFIG_T vi_config;
    TKL_VI_EXT_CONFIG_T ext_conf;

    ext_conf.type = TKL_VI_EXT_CONF_CAMERA;
    ext_conf.camera.camera_type = TKL_VI_CAMERA_TYPE_DVP;
    ext_conf.camera.fmt = TKL_CODEC_VIDEO_H264;
    ext_conf.camera.power_pin = TUYA_GPIO_NUM_51;
    ext_conf.camera.active_level = TUYA_GPIO_LEVEL_HIGH;
    ext_conf.camera.i2c.clk = TUYA_GPIO_NUM_13;
    ext_conf.camera.i2c.sda = TUYA_GPIO_NUM_15;

    vi_config.isp.width = 480;
    vi_config.isp.height = 480;
    vi_config.isp.fps = 15;

    vi_config.pdata = &ext_conf;

    tkl_vi_init(&vi_config, 0);

    TKL_VENC_CONFIG_T h264_config;
    // DVP:0; UVC:1
    h264_config.enable_h264_pipeline = 0; // dvp
    h264_config.put_cb = __h264_cb;
    tkl_venc_init(0, &h264_config, 0);
}

VOID example_dvp(INT_T argc, CHAR_T *argv[])
{
    OPERATE_RET rt = OPRT_OK;

    rt = tal_mutex_create_init(&sg_dvp_mutex);
    if (OPRT_OK != rt) {
        TAL_PR_ERR("tal_mutex_create_init failed, rt = %d", rt);
        return;
    }
    TAL_PR_DEBUG("DVP mutex created successfully");

    rt = app_fs_init();
    if (OPRT_OK != rt) {
        TAL_PR_ERR("app_fs_init failed, rt = %d", rt);
        return;
    }
    TAL_PR_DEBUG("File system initialized successfully");

    // Initialize the DVP driver
    example_dvp_init();
    TAL_PR_DEBUG("DVP driver initialized successfully");

__EXIT:
    return;
}

VOID example_dvp_stop(INT_T argc, CHAR_T *argv[])
{
    if (sg_dvp_mutex == NULL) {
        TAL_PR_ERR("DVP mutex is NULL");
        return;
    }

    tal_mutex_lock(sg_dvp_mutex);

    TKL_VENC_CONFIG_T h264_config;
    // DVP:0; UVC:1
    h264_config.enable_h264_pipeline = 0; // dvp
    h264_config.put_cb = __h264_cb;
    tkl_venc_uninit(0, &h264_config);

    tkl_vi_uninit(TKL_VI_CAMERA_TYPE_DVP);
    tal_mutex_unlock(sg_dvp_mutex);

    tal_mutex_release(sg_dvp_mutex);
    sg_dvp_mutex = NULL;

    tkl_fs_unmount(VIDEO_FILE_DIR);
    TAL_PR_DEBUG("DVP driver uninitialized and file system unmounted successfully");

    return;
}
