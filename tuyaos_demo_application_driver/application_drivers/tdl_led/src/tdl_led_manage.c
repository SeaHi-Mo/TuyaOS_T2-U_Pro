/**
 * @file tdl_led.c
 * @author www.tuya.com
 * @brief tdl_led module is used to
 * @version 1.0.0
 * @date 2022-05-16
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#include "tdl_led_manage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tal_memory.h"
#include "tal_sw_timer.h"
#include "tkl_gpio.h"
#include "tal_mutex.h"
#include "uni_log.h"
#include "tuya_list.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
// 调试打印
#if 1
#define TDL_LED_PRINT(fmt, ...)
#else
#define TDL_LED_PRINT(fmt, ...) PR_DEBUG(fmt, ##__VA_ARGS__)
#endif

#define LED_TIMER_VAL_MS 50 // 软件定时器运行周期，也就意味着设置的最小单位：50ms

#define TDL_LED_NAME_LEN 32
/***********************************************************
***********************typedef define***********************
***********************************************************/
// 运行状态
typedef enum {
    FLASH_NONE,
    FLASH_START,
    FLASH_FIRST,
    FLASH_SECOND,
} RUN_FLASH_STAT_E;

// 节点配置信息
typedef struct {
    TDL_LED_CONFIG_T cfg;          // 配置表
    CHAR_T *name;
    PVOID_T tdd_handle;            // led控制回调函数
    TDL_LED_CTRL_CB led_ctrl_cb;

    TDL_LED_STAT_E stat; // 当前状态
    // 闪烁配置
    RUN_FLASH_STAT_E run_stat; // 当前的闪烁状态
    UINT_T run_flash_cnt;      // 闪烁次数，亮灭算一次
    UINT_T run_first_time;     // 闪烁一次前半周期时间
    UINT_T run_second_time;    // 闪烁一次后半周期时间

    struct tuya_list_head node; // 节点
} TDL_LED_NODE_T;

// 互斥量配置
typedef struct {
    BOOL_T is_mutex_enable;
    MUTEX_HANDLE mutex;
} TDL_MUTEX_T;

// 指示灯管理结构
typedef struct {
    TIMER_ID led_tm;                     // 软件定时器
    TDL_MUTEX_T led_mutex;               // 互斥量管理
    struct tuya_list_head led_list_head; // 链表
} TDL_LED_MNG_T;

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC TDL_LED_MNG_T sg_led_mng = {
    .led_mutex.is_mutex_enable = TRUE, // 互斥锁默认打开
};

/***********************************************************
***********************function define**********************
***********************************************************/
/**
 * @brief 初始化互斥量，默认使用互斥量进行保护
 *
 * @return OPRT_OK 代表成功。如果是其他错误码，请参考 tuya_error_code.h
 */
STATIC OPERATE_RET __create_led_mutex_init(VOID_T)
{
    OPERATE_RET op_ret = OPRT_OK;
    if (sg_led_mng.led_mutex.is_mutex_enable) {
        op_ret = tal_mutex_create_init(&sg_led_mng.led_mutex.mutex);
    }
    return op_ret;
}

/**
 * @brief 设置互斥锁状态
 *
 * @param[in] en TRUE 上锁 FALSE 解锁
 * @return VOID_T
 */
STATIC VOID_T __set_mutex_lock(BOOL_T en)
{
    if (sg_led_mng.led_mutex.is_mutex_enable) {
        if (en) {
            tal_mutex_lock(sg_led_mng.led_mutex.mutex);
        } else {
            tal_mutex_unlock(sg_led_mng.led_mutex.mutex);
        }
    }

    return;
}

/**
 * @brief 遍历led句柄
 *
 * @param[in] handle led链表头
 * @return 查找成功：返回led链表 查找失败：返回NULL
 */
STATIC TDL_LED_NODE_T *__find_led_handler(PVOID_T handle)
{
    TDL_LED_NODE_T *entry = NULL;
    struct tuya_list_head *pos = NULL;
    tuya_list_for_each(pos, &sg_led_mng.led_list_head)
    {
        entry = tuya_list_entry(pos, TDL_LED_NODE_T, node);
        if (handle == entry->tdd_handle) { //对比TDD层handle
            return entry;
        }
    }
    return NULL;
}

/**
 * @brief 设置led状态
 *
 * @param[in] node 要操作的led句柄
 * @param[in] stat 要设置的状态
 * @return VOID_T
 */
STATIC VOID_T __set_led_stat(TDL_LED_NODE_T *node, TDL_LED_STAT_E stat)
{
    node->stat = stat;
    if (node->led_ctrl_cb && node->tdd_handle) {
       node->led_ctrl_cb(node->tdd_handle, stat);
    } else {
        PR_NOTICE("set led:%d, please register function!");
    }

    return;
}

/**
 * @brief 设置led闪烁状态
 *
 * @param[in] node 指示灯节点句柄
 * @return VOID_T
 */
STATIC VOID_T __set_led_flash_stat(TDL_LED_NODE_T *node)
{
    switch (node->run_stat) {
    case FLASH_NONE:
        // 输出首次电平
        TDL_LED_PRINT("start..");
        node->run_flash_cnt = node->cfg.flash_cnt;
        if (node->run_flash_cnt > 0) {
            node->run_stat = FLASH_FIRST;
            node->run_first_time = node->cfg.flash_first_time;
        }
        __set_led_stat(node, node->cfg.start_stat);
        break;

    case FLASH_FIRST:
        // 闪烁,上半周期
        if (node->run_first_time > 0) {
            node->run_first_time -= LED_TIMER_VAL_MS;
        }
        if (node->run_first_time == 0) {
            // 执行完成，设置下一状态
            if ((node->run_flash_cnt != TDL_FLASH_FOREVER) && (--node->run_flash_cnt == 0)) {
                // 闪烁次数为空，设置会结束状态
                goto FLASH_EXIT;
            } else {
                // 执行上半完成，设置下半周期
                TDL_LED_PRINT("[next 0]..%d", node->run_flash_cnt);
                node->run_stat = FLASH_SECOND;
                node->run_second_time = node->cfg.flash_second_time;
            }
            __set_led_stat(node, !node->cfg.start_stat);
        }
        break;

    case FLASH_SECOND:
        // 闪烁,下半周期
        if (node->run_second_time > 0) {
            node->run_second_time -= LED_TIMER_VAL_MS;
        }
        if (node->run_second_time == 0) {
            // 执行完成，设置下一状态
            if ((node->run_flash_cnt != TDL_FLASH_FOREVER) && (--node->run_flash_cnt == 0)) {
                // 闪烁次数为空，设置会结束状态
                goto FLASH_EXIT;
            } else {
                TDL_LED_PRINT("[next 1]..%d", node->run_flash_cnt);
                node->run_stat = FLASH_FIRST;
                node->run_first_time = node->cfg.flash_first_time;
            }
            __set_led_stat(node, node->cfg.start_stat);
        }
        break;

    default:
        break;
    }
    return;

FLASH_EXIT:
    TDL_LED_PRINT("[over]");
    node->run_stat = FLASH_NONE;
    node->cfg.stat = TDL_LED_FINISH;
    __set_led_stat(node, node->cfg.end_stat);

    return;
}

/**
 * @brief led状态控制
 *
 * @param[in] node 被控制的led句柄
 * @return VOID_T
 */
STATIC VOID_T __led_stat_controller(TDL_LED_NODE_T *node)
{
    // 状态相同不触发
    if (node->stat == node->cfg.stat) {
        return;
    }

    switch (node->cfg.stat) {
    case TDL_LED_OFF:
        __set_led_stat(node, TDL_LED_OFF);
        break;

    case TDL_LED_ON:
        __set_led_stat(node, TDL_LED_ON);
        break;

    case TDL_LED_FLASH:
        __set_led_flash_stat(node);
        break;

    default:
        break;
    }

    return;
}

/**
 * @brief 用于flash闪烁的软件定时器
 *
 * @param[in] timerID 定时器 id
 * @param[in] pTimerArg 参数，为 null
 * @return VOID_T
 */
STATIC VOID_T __led_timer_cb(IN TIMER_ID timerID, IN PVOID_T pTimerArg)
{
    // 遍历每个链表
    __set_mutex_lock(TRUE);
    TDL_LED_NODE_T *entry = NULL;
    struct tuya_list_head *pos = NULL;
    tuya_list_for_each(pos, &sg_led_mng.led_list_head)
    {
        entry = tuya_list_entry(pos, TDL_LED_NODE_T, node);
        __led_stat_controller(entry);
    }
    __set_mutex_lock(FALSE);

    return;
}

/**
 * @brief 设置指示灯互斥量
 *
 * @param[in] en TRUE - 使能(默认是使能无需调用)；  FALSE - 取消互斥量
 * @return OPRT_OK 代表成功。如果是其他错误码，请参考 tuya_error_code.h
 */
OPERATE_RET tdl_led_set_mutex_enable(IN BOOL_T en)
{
    sg_led_mng.led_mutex.is_mutex_enable = en;

    return OPRT_OK;
}

/**
 * @brief 对led进行控制
 *
 * @param[in] handle 句柄，通过句柄查找资源
 * @param[in] cfg 配置表
 * @return OPRT_OK 代表成功。如果是其他错误码，请参考 tuya_error_code.h
 */
OPERATE_RET tdl_led_ctrl(IN PVOID_T handle, IN TDL_LED_CONFIG_T *cfg)
{
    if (NULL == handle || NULL == cfg) {
        return OPRT_INVALID_PARM;
    }

    if (cfg->stat >= TDL_LED_FINISH) {
        return OPRT_INVALID_PARM;
    }

    TDL_LED_NODE_T *node = __find_led_handler(handle);
    if (NULL == node) {
        PR_ERR("led ctrl not find!");
        return OPRT_INVALID_PARM;
    }


    __set_mutex_lock(TRUE);
    // 配置cfg
    TDL_LED_STAT_E stat;
    if (cfg->stat == TDL_LED_FLASH) {
        cfg->flash_first_time = cfg->flash_first_time / LED_TIMER_VAL_MS * LED_TIMER_VAL_MS;
        cfg->flash_second_time = cfg->flash_second_time / LED_TIMER_VAL_MS * LED_TIMER_VAL_MS;
        // 清空运行态
        node->run_stat = FLASH_NONE;
    } else {
        stat = cfg->stat;
        memset(cfg, 0, SIZEOF(TDL_LED_CONFIG_T)); // 开关需要将其他参数清空，避免有歧义
        cfg->stat = stat;
        __set_led_stat(node, cfg->stat); // 立刻触发
    }
    memcpy(&node->cfg, cfg, SIZEOF(TDL_LED_CONFIG_T));
    __set_mutex_lock(FALSE);

    return OPRT_OK;
}

/**
 * @brief 通过设备名找到对应的handle
 *
 * @param[in] dev_name  led name
 * @param[out] handle 操作的句柄对象
 * @return 
*/
OPERATE_RET tdl_led_dev_find(IN CHAR_T *dev_name, INOUT PVOID_T *handle)
{
    if (NULL == dev_name) {
        return OPRT_INVALID_PARM;
    }
    
    TDL_LED_NODE_T *dev_node = NULL;
    struct tuya_list_head *pos = NULL;

    tuya_list_for_each(pos, &sg_led_mng.led_list_head)
    {
        dev_node = tuya_list_entry(pos, TDL_LED_NODE_T, node);
        if (0 == strcmp(dev_name, dev_node->name)) {
            *handle = dev_node->tdd_handle;
            return OPRT_OK;
        }
    }

    return OPRT_NOT_FOUND;
}


/**
 * @brief 查找设备名是否已存在
 *
 * @param[in] dev_name  led name
 * @return 
*/
STATIC OPERATE_RET __tdl_led_find_node_name(IN CHAR_T *dev_name)
{
    if (NULL == dev_name) {
        return OPRT_INVALID_PARM;
    }
    
    TDL_LED_NODE_T *dev_node = NULL;
    struct tuya_list_head *pos = NULL;

    tuya_list_for_each(pos, &sg_led_mng.led_list_head)
    {
        dev_node = tuya_list_entry(pos, TDL_LED_NODE_T, node);
        if (0 == strcmp(dev_name, dev_node->name)) {
            return OPRT_OK;
        }
    }

    return OPRT_NOT_FOUND;
}

/**
 * @brief 注册底层驱动实现，用于可无需关心具体驱动实现，只需关心控制对象
 *
 * @param[in] dev_name  led name
 * @param[in] handle 操作的句柄对象
 * @param[in] cb led驱动实现
 * @return OPRT_OK 代表成功。如果是其他错误码，请参考 tuya_error_code.h
 */
OPERATE_RET tdl_led_register_function(IN CHAR_T *dev_name, IN PVOID_T handle, IN TDL_LED_CTRL_CB cb)
{
    if ((NULL == dev_name) || (NULL == handle) || (NULL == cb)) {
        return OPRT_INVALID_PARM;
    }

    OPERATE_RET op_ret = OPRT_OK;
    UCHAR_T name_len = 0;

    STATIC BOOL_T is_init = FALSE;
    if (!is_init) {
        // 互斥信号量初始化
        op_ret = __create_led_mutex_init();
        if (OPRT_OK != op_ret) {
            PR_ERR("create led mutex err:%d", op_ret);
            return op_ret;
        }

        // 创建用于闪烁的软件定时器
        op_ret = tal_sw_timer_create(__led_timer_cb, NULL, &sg_led_mng.led_tm);
        if (OPRT_OK != op_ret) {
            PR_ERR("add led timer err:%d", op_ret);
            return op_ret;
        }
        op_ret = tal_sw_timer_start(sg_led_mng.led_tm, LED_TIMER_VAL_MS, TAL_TIMER_CYCLE);
        if (OPRT_OK != op_ret) {
            PR_ERR("start led timer err:%d", op_ret);
            return op_ret;
        }

        // 初始化头链表
        INIT_LIST_HEAD(&sg_led_mng.led_list_head);

        is_init = TRUE;
        PR_DEBUG("tdl led manage init");
    }

    if(OPRT_OK == __tdl_led_find_node_name(dev_name)){
        PR_NOTICE("led name existence");
        return OPRT_INIT_MORE_THAN_ONCE;
    }

    // 申请空间
    TDL_LED_NODE_T *p_node = (TDL_LED_NODE_T *)tal_malloc(SIZEOF(TDL_LED_NODE_T));
    if (NULL == p_node) {
        PR_ERR("TDL LED INIT MALLOC ERROR");
        return OPRT_MALLOC_FAILED;
    }
    memset(p_node, 0, SIZEOF(TDL_LED_NODE_T));

    //创建新名称
    p_node->name = (CHAR_T* )tal_malloc(TDL_LED_NAME_LEN);
    if(NULL == p_node->name){
        tal_free(p_node);
        p_node = NULL;
        return OPRT_MALLOC_FAILED;
    }
    memset(p_node->name, 0, TDL_LED_NAME_LEN);

    name_len = strlen(dev_name);
    if(name_len >= TDL_LED_NAME_LEN){
        name_len = TDL_LED_NAME_LEN;
    }
    memcpy(p_node->name, dev_name, name_len);

    p_node->stat = -1;
    p_node->tdd_handle = handle;
    p_node->led_ctrl_cb = cb;

    // 加入head链表中
    __set_mutex_lock(TRUE);
    tuya_list_add(&p_node->node, &sg_led_mng.led_list_head);
    __set_mutex_lock(FALSE);

    return OPRT_OK;
}