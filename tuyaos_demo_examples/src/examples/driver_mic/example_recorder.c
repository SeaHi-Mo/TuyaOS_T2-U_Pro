/**
 * @file example_recorder.c
 * @brief example_recorder module is used to 
 * @version 0.1
 * @date 2025-02-20
 */

#include "tuya_ringbuf.h"

#include "tal_thread.h"
#include "tal_system.h"
#include "tal_log.h"

#include "tkl_fs.h"
#include "tkl_memory.h"
#include "tkl_audio.h"
#include "tkl_gpio.h"

#include "wav_encode.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define USE_RING_BUFFER         0
#define USE_INTERNAL_FLASH      1 // 将录音文件放到内部 flash 中
#define USE_SD_CARD             2
#define RECORDER_FILE_SOURCE    USE_RING_BUFFER

#if (RECORDER_FILE_SOURCE == USE_INTERNAL_FLASH)
#define RECORDER_FILE_DIR       "/"
#define RECORDER_FILE_PATH      "/tuya_recorder.pcm"
#define RECORDER_WAV_FILE_PATH  "/tuya_recorder.wav"
#elif (RECORDER_FILE_SOURCE == USE_SD_CARD)
#define RECORDER_FILE_DIR       "/sdcard"
#define RECORDER_FILE_PATH      "/sdcard/tuya_recorder.pcm"
#define RECORDER_WAV_FILE_PATH  "/sdcard/tuya_recorder.wav"
#endif

#define SPEAKER_ENABLE_PIN  TUYA_GPIO_NUM_28
#define AUDIO_TRIGGER_PIN   TUYA_GPIO_NUM_12

// MIC 采样率
#define MIC_SAMPLE_RATE TKL_AUDIO_SAMPLE_16K
// MIC 采样位数
#define MIC_SAMPLE_BITS TKL_AUDIO_DATABITS_16
// MIC 通道
#define MIC_CHANNEL TKL_AUDIO_CHANNEL_MONO
// 最大可缓存的录音时长，单位ms
#define MIC_RECORD_DURATION_MS (3 * 1000)
// RINGBUF 大小
#define PCM_BUF_SIZE (MIC_RECORD_DURATION_MS * MIC_SAMPLE_RATE * MIC_SAMPLE_BITS * MIC_CHANNEL / 8 / 1000)
// 10ms 的 PCM 数据大小
#define PCM_FRAME_SIZE  (10 * MIC_SAMPLE_RATE * MIC_SAMPLE_BITS * MIC_CHANNEL / 8 / 1000)

/***********************************************************
***********************typedef define***********************
***********************************************************/
struct recorder_ctx {
    TUYA_RINGBUFF_T pcm_buf;

    // 录音状态
    BOOL_T recording;

    // 播放状态
    BOOL_T playing;

    // 录音文件句柄
    TUYA_FILE file_hdl;
};

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC THREAD_HANDLE recorder_hdl = NULL;

struct recorder_ctx sg_recorder_ctx = {
    .pcm_buf = NULL,
    .recording = FALSE,
    .playing = FALSE,
    .file_hdl = NULL,
};

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC VOID_T app_audio_trigger_pin_init(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_GPIO_BASE_CFG_T pin_cfg = {
        .mode = TUYA_GPIO_PULLUP,
        .direct = TUYA_GPIO_INPUT,
        .level = TUYA_GPIO_LEVEL_HIGH,
    };
    TUYA_CALL_ERR_LOG(tkl_gpio_init(AUDIO_TRIGGER_PIN, &pin_cfg));

    return;
}

STATIC BOOL_T audio_trigger_pin_is_pressed(VOID_T)
{
    TUYA_GPIO_LEVEL_E level = TUYA_GPIO_LEVEL_HIGH;
    tkl_gpio_read(AUDIO_TRIGGER_PIN, &level);
    return (level == TUYA_GPIO_LEVEL_LOW) ? TRUE : FALSE;
}

STATIC INT_T _audio_frame_put(TKL_AUDIO_FRAME_INFO_T *pframe)
{
    if (NULL == sg_recorder_ctx.pcm_buf) {
        return pframe->buf_size;
    }

    UINT32_T free_size = tuya_ring_buff_free_size_get(sg_recorder_ctx.pcm_buf);
    if (sg_recorder_ctx.recording && free_size >= pframe->buf_size) {
        tuya_ring_buff_write(sg_recorder_ctx.pcm_buf, pframe->pbuf, pframe->buf_size);
    }

    return pframe->buf_size;
}

STATIC VOID_T app_audio_init(VOID_T)
{
    TKL_AUDIO_CONFIG_T config;

    memset(&config, 0, sizeof(TKL_AUDIO_CONFIG_T));

    config.enable = 1;
    config.ai_chn = 0;
    config.sample = MIC_SAMPLE_RATE;
    config.spk_sample = MIC_SAMPLE_RATE; // 与MIC采样率一致，这样可以避免音频数据转换，直接播放 MIC 采集的音频数据
    config.datebits = MIC_SAMPLE_BITS;
    config.channel = MIC_CHANNEL;
    config.codectype = TKL_CODEC_AUDIO_PCM;
    config.card = TKL_AUDIO_TYPE_BOARD;
    config.put_cb = _audio_frame_put;
    config.spk_gpio = SPEAKER_ENABLE_PIN;
    config.spk_gpio_polarity = 0;

    tkl_ai_init(&config, 0);

    tkl_ai_start(0, 0);

    tkl_ai_set_vol(0, 0, 80);

    tkl_ao_set_vol(TKL_AUDIO_TYPE_BOARD, 0, NULL, 30);

    return;
}

STATIC VOID_T app_mic_record(VOID_T)
{
#if RECORDER_FILE_SOURCE == USE_RING_BUFFER
    // Nothing to do
    // 通过 ring buffer 记录只会记录前 MIC_RECORD_DURATION_MS 的数据
    // 不会记录后面的数据，这样是为了避免 ring buffer 中对后面数据的覆盖，导致破坏了 PCM 格式，播放时出现异常
#elif (RECORDER_FILE_SOURCE == USE_INTERNAL_FLASH) || (RECORDER_FILE_SOURCE == USE_SD_CARD)
    if (NULL == sg_recorder_ctx.file_hdl) {
        return;
    }

    UINT32_T data_len = 0;
    data_len = tuya_ring_buff_used_size_get(sg_recorder_ctx.pcm_buf);
    if (data_len == 0) {
        return;
    }

    CHAR_T * read_buf = tkl_system_psram_malloc(data_len);
    if (NULL == read_buf) {
        TAL_PR_ERR("tkl_system_psram_malloc failed");
        return;
    }

    // 写入文件
    tuya_ring_buff_read(sg_recorder_ctx.pcm_buf, read_buf, data_len);
    INT_T rt_len = tkl_fwrite(read_buf, data_len, sg_recorder_ctx.file_hdl);
    if (rt_len != data_len) {
        TAL_PR_ERR("write file failed, maybe disk full");
        TAL_PR_ERR("write len %d, data len %d", rt_len, data_len);
    }

    if (read_buf) {
        tkl_system_psram_free(read_buf);
        read_buf = NULL;
    }
#endif
    return;
}

#if RECORDER_FILE_SOURCE == USE_RING_BUFFER
STATIC VOID_T app_recode_play_from_ringbuf(VOID_T)
{
    if (NULL == sg_recorder_ctx.pcm_buf) {
        return;
    }

    UINT32_T data_len = tuya_ring_buff_used_size_get(sg_recorder_ctx.pcm_buf);
    if (data_len == 0) {
        return;
    }

    UINT32_T out_len = 0;
    CHAR_T * frame_buf = tkl_system_psram_malloc(PCM_FRAME_SIZE);
    if (NULL == frame_buf) {
        TAL_PR_ERR("tkl_system_psram_malloc failed");
        return;
    }

    do {
        memset(frame_buf, 0, PCM_FRAME_SIZE);
        out_len = 0;

        data_len = tuya_ring_buff_used_size_get(sg_recorder_ctx.pcm_buf);
        if (data_len == 0) {
            break;
        }

        if (data_len > PCM_FRAME_SIZE) {
            tuya_ring_buff_read(sg_recorder_ctx.pcm_buf, frame_buf, PCM_FRAME_SIZE);
            out_len = PCM_FRAME_SIZE;
        } else {
            tuya_ring_buff_read(sg_recorder_ctx.pcm_buf, frame_buf, data_len);
            out_len = data_len;
        }

        TKL_AUDIO_FRAME_INFO_T frame_info;
        frame_info.pbuf = frame_buf;
        frame_info.used_size = out_len;
        tkl_ao_put_frame(0, 0, NULL, &frame_info);
    } while (1);

    if (frame_buf) {
        tkl_system_psram_free(frame_buf);
        frame_buf = NULL;
    }
}
#endif

#if (RECORDER_FILE_SOURCE == USE_INTERNAL_FLASH) || (RECORDER_FILE_SOURCE == USE_SD_CARD)
STATIC VOID_T app_recode_play_from_flash(VOID_T)
{
    // 读取文件
    TUYA_FILE file_hdl = tkl_fopen(RECORDER_FILE_PATH, "r");
    if (NULL == file_hdl) {
        TAL_PR_ERR("open file failed");
        return;
    }

    UINT32_T data_len = 0;
    CHAR_T * read_buf = tkl_system_psram_malloc(PCM_FRAME_SIZE);
    if (NULL == read_buf) {
        TAL_PR_ERR("tkl_system_psram_malloc failed");
        return;
    }

    do {
        memset(read_buf, 0, PCM_FRAME_SIZE);
        data_len = tkl_fread(read_buf, PCM_FRAME_SIZE, file_hdl);
        if (data_len == 0) {
            break;
        }

        TKL_AUDIO_FRAME_INFO_T frame_info;
        frame_info.pbuf = read_buf;
        frame_info.used_size = data_len;
        tkl_ao_put_frame(0, 0, NULL, &frame_info);
    } while (1);

    if (read_buf) {
        tkl_system_psram_free(read_buf);
        read_buf = NULL;
    }

    if (file_hdl) {
        tkl_fclose(file_hdl);
        file_hdl = NULL;
    }
}
#endif

STATIC VOID_T app_recode_play(VOID_T)
{
#if RECORDER_FILE_SOURCE == USE_RING_BUFFER
    app_recode_play_from_ringbuf();
#elif (RECORDER_FILE_SOURCE == USE_INTERNAL_FLASH) || (RECORDER_FILE_SOURCE == USE_SD_CARD)
    app_recode_play_from_flash();
#endif
    return;
}

#if (RECORDER_FILE_SOURCE == USE_SD_CARD)
STATIC OPERATE_RET app_pcm_to_wav(CHAR_T *pcm_file)
{
    OPERATE_RET rt = OPRT_OK;

    UINT8_T head[WAV_HEAD_LEN] = {0};
    UINT32_T pcm_len = 0;
    UINT32_T sample_rate = MIC_SAMPLE_RATE;
    UINT16_T bit_depth = MIC_SAMPLE_BITS;
    UINT16_T channel = MIC_CHANNEL;

    // 获取 pcm 文件长度
    TUYA_FILE pcm_hdl = tkl_fopen(pcm_file, "r");
    if (NULL == pcm_hdl) {
        TAL_PR_ERR("open file failed");
        return OPRT_FILE_OPEN_FAILED;
    }
    tkl_fseek(pcm_hdl, 0, 2);
    pcm_len = tkl_ftell(pcm_hdl);

    tkl_fclose(pcm_hdl);
    pcm_hdl = NULL;

    TAL_PR_DEBUG("pcm file len %d", pcm_len);
    if (pcm_len == 0) {
        TAL_PR_ERR("pcm file is empty");
        return OPRT_COM_ERROR;
    }

    // 获取 wav head
    rt = app_get_wav_head(pcm_len, 1, sample_rate, bit_depth, channel, head);
    if (OPRT_OK != rt) {
        TAL_PR_ERR("app_get_wav_head failed, rt = %d", rt);
        return rt;
    }

    TAL_PR_HEXDUMP_DEBUG("wav head", head, WAV_HEAD_LEN);

    // 创建 wav 文件
    TUYA_FILE wav_hdl = tkl_fopen(RECORDER_WAV_FILE_PATH, "w");
    if (NULL == wav_hdl) {
        TAL_PR_ERR("open file: %s failed", RECORDER_WAV_FILE_PATH);
        rt = OPRT_FILE_OPEN_FAILED;
        goto __EXIT;
    }

    // 写 wav head
    tkl_fwrite(head, WAV_HEAD_LEN, wav_hdl);

    // 读取 pcm 文件
    CHAR_T * read_buf = tkl_system_psram_malloc(PCM_FRAME_SIZE);
    if (NULL == read_buf) {
        TAL_PR_ERR("tkl_system_psram_malloc failed");
        // return OPRT_COM_ERROR;
        rt = OPRT_MALLOC_FAILED;
        goto __EXIT;
    }

    TAL_PR_DEBUG("pcm file len %d", pcm_len);
    pcm_hdl = tkl_fopen(pcm_file, "r");
    if (NULL == pcm_hdl) {
        TAL_PR_ERR("open file failed");
        rt = OPRT_FILE_OPEN_FAILED;
        goto __EXIT;
    }

    tkl_fseek(pcm_hdl, WAV_HEAD_LEN, 0);

    // 写 wav data
    do {
        memset(read_buf, 0, PCM_FRAME_SIZE);
        UINT32_T read_len = tkl_fread(read_buf, PCM_FRAME_SIZE, pcm_hdl);
        if (read_len == 0) {
            break;
        }

        tkl_fwrite(read_buf, read_len, wav_hdl);
    } while (1);

__EXIT:
    if (pcm_hdl) {
        tkl_fclose(pcm_hdl);
        pcm_hdl = NULL;
    }

    if (wav_hdl) {
        tkl_fclose(wav_hdl);
        wav_hdl = NULL;
    }

    if (read_buf) {
        tkl_system_psram_free(read_buf);
        read_buf = NULL;
    }

    return rt;
}
#endif

STATIC VOID_T app_recorder_thread(VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;

    app_audio_trigger_pin_init();
    app_audio_init();

    for (;;) {
        app_mic_record();

        if (FALSE == audio_trigger_pin_is_pressed()) {
            tal_system_sleep(100);

            // 结束收音
            if (TRUE == sg_recorder_ctx.recording) {
                sg_recorder_ctx.recording = FALSE;
                sg_recorder_ctx.playing = TRUE;
#if (RECORDER_FILE_SOURCE == USE_INTERNAL_FLASH) || (RECORDER_FILE_SOURCE == USE_SD_CARD)
                if (sg_recorder_ctx.file_hdl) {
                    tkl_fclose(sg_recorder_ctx.file_hdl);
                    sg_recorder_ctx.file_hdl = NULL;
                }
                // pcm 转 wav
#if (RECORDER_FILE_SOURCE == USE_SD_CARD)
                rt = app_pcm_to_wav(RECORDER_FILE_PATH);
                if (OPRT_OK != rt) {
                    TAL_PR_ERR("app_pcm_to_wav failed, rt = %d", rt);
                }
#endif
#endif
                TAL_PR_DEBUG("stop recording");
            }

            // 开始播放
            if (TRUE == sg_recorder_ctx.playing) {
                TAL_PR_DEBUG("start playing");
                sg_recorder_ctx.playing = FALSE;
                app_recode_play();
                TAL_PR_DEBUG("stop playing");
            }

            continue;
        }

        // 开始收音
        if (FALSE == sg_recorder_ctx.recording) {
#if (RECORDER_FILE_SOURCE == USE_INTERNAL_FLASH) || (RECORDER_FILE_SOURCE == USE_SD_CARD)
            // 存在录音文件，删除
            BOOL_T is_exist = FALSE;
            tkl_fs_is_exist(RECORDER_FILE_PATH, &is_exist);
            if (is_exist == TRUE) {
                tkl_fs_remove(RECORDER_FILE_PATH);
                TAL_PR_DEBUG("remove file %s", RECORDER_FILE_PATH);
            }

            is_exist = FALSE;
            tkl_fs_is_exist(RECORDER_WAV_FILE_PATH, &is_exist);
            if (is_exist == TRUE) {
                tkl_fs_remove(RECORDER_WAV_FILE_PATH);
                TAL_PR_DEBUG("remove file %s", RECORDER_WAV_FILE_PATH);
            }

            // 创建录音文件
            sg_recorder_ctx.file_hdl = tkl_fopen(RECORDER_FILE_PATH, "w");
            if (NULL == sg_recorder_ctx.file_hdl) {
                TAL_PR_ERR("open file failed");
                continue;
            }
            TAL_PR_DEBUG("open file %s success", RECORDER_FILE_PATH);
#endif
            tuya_ring_buff_reset(sg_recorder_ctx.pcm_buf);
            sg_recorder_ctx.recording = TRUE;
            sg_recorder_ctx.playing = FALSE;
            TAL_PR_DEBUG("start recording");
        }

        tal_system_sleep(10);
    }
}

STATIC OPERATE_RET app_fs_init(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

#if RECORDER_FILE_SOURCE == USE_INTERNAL_FLASH
    rt = tkl_fs_mount("/", DEV_INNER_FLASH);
    if (rt != OPRT_OK) {
        TAL_PR_ERR("mount fs failed ");
        return rt;
    }
    TAL_PR_DEBUG("mount inner flash success ");
#elif RECORDER_FILE_SOURCE == USE_SD_CARD
    rt = tkl_fs_mount("/sdcard", DEV_SDCARD);
    if (rt != OPRT_OK) {
        TAL_PR_ERR("mount sd card failed, please retry after format ");
        return rt;
    }
    TAL_PR_DEBUG("mount sd card success ");
#else
    return rt;
#endif

    return rt;
}

VOID_T example_recorder_init(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    // 初始化文件系统
    rt = app_fs_init();
    if (OPRT_OK != rt) {
        TAL_PR_ERR("app_fs_init failed, rt = %d", rt);
        return;
    }

    // 创建 PCM buffer
    if (NULL == sg_recorder_ctx.pcm_buf) {
        TAL_PR_DEBUG("create pcm buffer size %d", PCM_BUF_SIZE);
        rt = tuya_ring_buff_create(PCM_BUF_SIZE, OVERFLOW_PSRAM_STOP_TYPE, &sg_recorder_ctx.pcm_buf);
        if (OPRT_OK != rt) {
            TAL_PR_ERR("tuya_ring_buff_create failed, rt = %d", rt);
            return;
        }
    }

    THREAD_CFG_T thrd_param = {1024*6, THREAD_PRIO_3, "recorder task"};
    tal_thread_create_and_start(&recorder_hdl, NULL, NULL, app_recorder_thread, NULL, &thrd_param);
    return;
}
