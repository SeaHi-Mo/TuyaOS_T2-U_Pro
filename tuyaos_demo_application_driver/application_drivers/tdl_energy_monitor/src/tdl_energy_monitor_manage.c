/**
 * @file tdl_energy_monitor_manage.c
 * @author www.tuya.com
 * @brief tdl_energy_monitor_manage module is used to 
 * @version 0.1
 * @date 2023-07-13
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tdl_energy_monitor_manage.h"

#include "tal_log.h"
#include "tal_mutex.h"
#include "tal_memory.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define GET_ERR_PERCENT(cal, def) (((cal>def) ? (cal-def) : (def-cal)) * 100 / def)

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct energy_monitor_node {
    struct energy_monitor_node *next;
    CHAR_T name[ENERGY_MONITOR_NAME_LEN_MAX+1];
    UINT8_T is_open;

    TDD_EM_DRV_HANDLE tdd_hdl;
    ENERGY_DRV_INTFS_T drv_intfs;

    MUTEX_HANDLE mutex_hdl;

    UINT32_T energy_consumed;
    ENERGY_MONITOR_NOTIFY_CALLBACK notify_cb;
} ENERGY_MONITOR_NODE_T;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC ENERGY_MONITOR_NODE_T sg_energy_head = {
    .next = NULL,
};

/***********************************************************
***********************function define**********************
***********************************************************/

ENERGY_MONITOR_NODE_T* __energy_monitor_node_find(CHAR_T *name)
{
    ENERGY_MONITOR_NODE_T *node = sg_energy_head.next;

    while (node) {
        if (0 == strcmp(name, node->name)) {
            return node;
        }
        node = node->next;
    }

    return NULL;
}

OPERATE_RET tdl_energy_monitor_find(CHAR_T *name, ENERGY_MONITOR_HANDLE_T *handle)
{
    OPERATE_RET rt = OPRT_OK;
    ENERGY_MONITOR_NODE_T *node = NULL;

    TUYA_CHECK_NULL_RETURN(name, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);

    node = __energy_monitor_node_find(name);
    if (NULL == node) {
        TAL_PR_ERR("energy monitor %s not find", name);
        return OPRT_NOT_FOUND;
    }

    *handle = (ENERGY_MONITOR_HANDLE_T)node;

    return rt;
}

OPERATE_RET tdl_energy_monitor_open(ENERGY_MONITOR_HANDLE_T handle)
{
    OPERATE_RET rt = OPRT_OK;
    ENERGY_MONITOR_NODE_T *node = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);

    node = (ENERGY_MONITOR_NODE_T *)handle;

    if (node->is_open) {
        return OPRT_OK;
    }

    TUYA_CHECK_NULL_RETURN(node->drv_intfs.open, OPRT_COM_ERROR);

    // open tdd
    TUYA_CALL_ERR_RETURN(node->drv_intfs.open(node->tdd_hdl));

    TUYA_CALL_ERR_RETURN(tal_mutex_create_init(&node->mutex_hdl));

    node->is_open = 1;

    return rt;
}

OPERATE_RET tdl_energy_monitor_read_vcp(ENERGY_MONITOR_HANDLE_T handle, ENERGY_MONITOR_VCP_T *vcp_data)
{
    OPERATE_RET rt = OPRT_OK;
    ENERGY_MONITOR_NODE_T *node = NULL;
    TDD_ENERGY_MONITOR_DATA_T energy_data = {0};

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(vcp_data, OPRT_INVALID_PARM);

    node = (ENERGY_MONITOR_NODE_T *)handle;

    tal_mutex_lock(node->mutex_hdl);

    TUYA_CALL_ERR_GOTO(node->drv_intfs.read(node->tdd_hdl, &energy_data), __EXIT);

    node->energy_consumed += energy_data.energy_consumed;

    vcp_data->voltage = energy_data.voltage;
    vcp_data->current = energy_data.current;
    vcp_data->power = energy_data.power;

    // TAL_PR_DEBUG("voltage: %d, current: %d, power: %d, energy_consumed:%d", energy_data.voltage, energy_data.current, energy_data.power, energy_data.energy_consumed);

    if (0 == vcp_data->power || 0 == vcp_data->current) {
        vcp_data->current = 0;
        vcp_data->power = 0;
    }

__EXIT:
    tal_mutex_unlock(node->mutex_hdl);

    return rt;
}

OPERATE_RET tdl_energy_monitor_read_energy(ENERGY_MONITOR_HANDLE_T handle, UINT32_T *energy_consumed)
{
    OPERATE_RET rt = OPRT_OK;
    ENERGY_MONITOR_NODE_T *node = NULL;
    TDD_ENERGY_MONITOR_DATA_T energy_data = {0};

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(energy_consumed, OPRT_INVALID_PARM);

    node = (ENERGY_MONITOR_NODE_T *)handle;

    tal_mutex_lock(node->mutex_hdl);

    TUYA_CALL_ERR_GOTO(node->drv_intfs.read(node->tdd_hdl, &energy_data), __EXIT);

    node->energy_consumed += energy_data.energy_consumed;

    *energy_consumed = node->energy_consumed;
    node->energy_consumed = 0;

    // 功率为 0 时，电流也应为 0
    if (energy_data.power == 0) {
        energy_data.current = 0;
    }

__EXIT:
    tal_mutex_unlock(node->mutex_hdl);

    return rt;
}

OPERATE_RET tdl_energy_monitor_config(ENERGY_MONITOR_HANDLE_T handle, TDL_ENERGY_MONITOR_CMD_E cmd, void *args)
{
    OPERATE_RET rt = OPRT_OK;
    ENERGY_MONITOR_NODE_T *node = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);

    node = (ENERGY_MONITOR_NODE_T *)handle;

    switch (cmd) {
        case (TDL_EM_CMD_CAL_DATA_SET): {
            TUYA_CHECK_NULL_RETURN(args, OPRT_INVALID_PARM);
            TUYA_CALL_ERR_RETURN(node->drv_intfs.config(node->tdd_hdl, TDD_ENERGY_CMD_CAL_DATA_SET, args));
        } break;
        case (TDL_EM_CMD_CAL_PARAMS_SET): {
            TUYA_CHECK_NULL_RETURN(args, OPRT_INVALID_PARM);
            TUYA_CALL_ERR_RETURN(node->drv_intfs.config(node->tdd_hdl, TDD_ENERGY_CMD_CAL_PARAMS_SET, args));
        } break;
        default : break;
    }

    return rt;
}

OPERATE_RET tdl_energy_monitor_close(ENERGY_MONITOR_HANDLE_T handle)
{
    OPERATE_RET rt = OPRT_OK;
    ENERGY_MONITOR_NODE_T *node = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);

    node = (ENERGY_MONITOR_NODE_T *)handle;

    TUYA_CALL_ERR_RETURN(node->drv_intfs.close(node->tdd_hdl));
    node->energy_consumed = 0;

    return rt;
}

OPERATE_RET tdl_energy_monitor_driver_register(IN CHAR_T *name, IN ENERGY_DRV_INTFS_T *intfs, IN TDD_EM_DRV_HANDLE tdd_hdl)
{
    OPERATE_RET rt = OPRT_OK;
    ENERGY_MONITOR_NODE_T *list_tail = &sg_energy_head;
    ENERGY_MONITOR_NODE_T *new_node = NULL;

    TUYA_CHECK_NULL_RETURN(name, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(intfs, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(intfs->open, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(intfs->read, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(intfs->config, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(intfs->close, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(tdd_hdl, OPRT_INVALID_PARM);

    if (NULL != __energy_monitor_node_find(name)) {
        TAL_PR_ERR("%s is already exists", name);
        return OPRT_COM_ERROR;
    }

    while(NULL != list_tail->next) {
        list_tail = list_tail->next;
    }

    new_node = tal_malloc(sizeof(ENERGY_MONITOR_NODE_T));
    TUYA_CHECK_NULL_GOTO(new_node, __ERR);
    memset(new_node, 0, sizeof(ENERGY_MONITOR_NODE_T));
    new_node->next = NULL;

    list_tail->next = new_node;
    strncpy(new_node->name, name, ENERGY_MONITOR_NAME_LEN_MAX);
    memcpy(&new_node->drv_intfs, intfs, sizeof(ENERGY_DRV_INTFS_T));
    new_node->tdd_hdl = tdd_hdl;

    return rt;

__ERR:
    if (NULL != new_node) {
        tal_free(new_node);
        new_node = NULL;
    }

    return rt;
}

UINT32_T tdl_energy_monitor_calibration(ENERGY_MONITOR_HANDLE_T handle, ENERGY_MONITOR_CAL_DATA_T cal_data, OUT ENERGY_MONITOR_CAL_PARAMS_T *cal_params)
{
    OPERATE_RET rt = OPRT_OK;
    UINT32_T err_percent = 0, tmp_err_percent=0;
    ENERGY_MONITOR_CAL_DATA_T def_data = {0};
    ENERGY_MONITOR_CAL_PARAMS_T def_params = {0}, exp_params = {0};
    ENERGY_MONITOR_NODE_T *node = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(cal_params, OPRT_INVALID_PARM);
    if (0 == cal_data.voltage || 0 == cal_data.current|| 0 == cal_data.power || 0 == cal_data.resval) {
        return OPRT_INVALID_PARM;
    }

    node = (ENERGY_MONITOR_NODE_T *)handle;
    TUYA_CHECK_NULL_RETURN(node->drv_intfs.config, OPRT_INVALID_PARM);


    TUYA_CALL_ERR_RETURN(node->drv_intfs.config(node->tdd_hdl, TDD_ENERGY_CMD_CAL_DATA_SET, &cal_data));
    TUYA_CALL_ERR_RETURN(node->drv_intfs.config(node->tdd_hdl, TDD_ENERGY_CMD_CAL_PARAMS_GET, cal_params));

    TAL_PR_DEBUG("cal_params: %d, %d, %d, %d", cal_params->voltage_period, cal_params->current_period, cal_params->power_period, cal_params->pf_num);

    TUYA_CALL_ERR_GOTO(node->drv_intfs.config(node->tdd_hdl, TDD_ENERGY_CMD_DEF_DATA_GET, &def_data), __EXIT);
    TUYA_CALL_ERR_GOTO(node->drv_intfs.config(node->tdd_hdl, TDD_ENERGY_CMD_DEF_PARAMS_GET, &def_params), __EXIT);

    exp_params.voltage_period = def_data.voltage * def_params.voltage_period / cal_data.voltage;
    exp_params.current_period = def_data.current * def_params.current_period / cal_data.current / cal_data.resval;
    exp_params.power_period   = def_data.power * def_params.power_period / cal_data.power / cal_data.resval;
    exp_params.pf_num = def_params.pf_num * cal_data.resval;

#if 1
    TAL_PR_DEBUG("--->");
    TAL_PR_DEBUG("---> voltage_period: def: %-6u, exp: %-6u, cal : %u", def_params.voltage_period, exp_params.voltage_period, cal_params->voltage_period);
    TAL_PR_DEBUG("---> current_period: def: %-6u, exp: %-6u, cal : %u", def_params.current_period, exp_params.current_period, cal_params->current_period);
    TAL_PR_DEBUG("---> power_period:   def: %-6u, exp: %-6u, cal : %u", def_params.power_period,   exp_params.power_period,   cal_params->power_period);
    TAL_PR_DEBUG("---> pf number:      def: %-6u, exp: %-6u, cal : %u", def_params.pf_num,   exp_params.pf_num,   cal_params->pf_num);
#endif

    TAL_PR_DEBUG("--->");
    err_percent = GET_ERR_PERCENT(cal_params->voltage_period, exp_params.voltage_period);
    TAL_PR_DEBUG("---> voltage_period error percent: %d", err_percent);
    tmp_err_percent = GET_ERR_PERCENT(cal_params->current_period, exp_params.current_period);
    TAL_PR_DEBUG("---> current_period error percent: %d", tmp_err_percent);
    if (err_percent < tmp_err_percent) {
        err_percent = tmp_err_percent;
    }
    tmp_err_percent = GET_ERR_PERCENT(cal_params->power_period, exp_params.power_period);
    TAL_PR_DEBUG("---> power_period   error percent: %d", tmp_err_percent);
    if (err_percent < tmp_err_percent) {
        err_percent = tmp_err_percent;
    }
    tmp_err_percent = GET_ERR_PERCENT(cal_params->pf_num, exp_params.pf_num);
    TAL_PR_DEBUG("---> pf number      error percent: %d", tmp_err_percent);
    if (err_percent < tmp_err_percent) {
        err_percent = tmp_err_percent;
    }
    TAL_PR_DEBUG("--->");

__EXIT:
    return err_percent;
}
