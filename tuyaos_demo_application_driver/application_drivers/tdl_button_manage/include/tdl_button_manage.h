/**
 * @file tdl_button_manage.h
 * @author franky.lin@tuya.com
 * @brief tdl_button_manage, base timer、semaphore、task
 * @version 1.0
 * @date 2022-03-20
 * @copyright Copyright (c) tuya.inc 2022
 * button trigger management component
 */

#ifndef _TDL_BUTTON_MANAGE_H_
#define _TDL_BUTTON_MANAGE_H_

#include "tuya_cloud_types.h"
#include "tdl_button_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/
typedef VOID* TDL_BUTTON_HANDLE;

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef enum{
	TDL_BUTTON_PRESS_DOWN = 0, 	    //按下触发
	TDL_BUTTON_PRESS_UP,	   	      //松开触发
  TDL_BUTTON_PRESS_SINGLE_CLICK,  //单击触发
	TDL_BUTTON_PRESS_DOUBLE_CLICK,  //双击触发
  TDL_BUTTON_PRESS_REPEAT,        //多击触发 
	TDL_BUTTON_LONG_PRESS_START,    //长按开始触发
	TDL_BUTTON_LONG_PRESS_HOLD,	    //长按保持触发
	TDL_BUTTON_PRESS_MAX,		        //无
  TDL_BUTTON_PRESS_NONE,          //无
}TDL_BUTTON_TOUCH_EVENT_E;//按键触发事件


typedef struct{
  USHORT_T long_start_vaild_time;       //按键长按开始有效时间(ms)：e.g  3000-长按3s触发
  USHORT_T long_keep_timer;             //按键长按持续触发时间(ms)：e.g 100ms-长按时每100ms触发一次
  USHORT_T button_debounce_time;        //消抖时间(ms)
  UCHAR_T  button_repeat_vaild_count;   //多击触发次数,大于2触发多击事件
  USHORT_T button_repeat_vaild_time;    //双击、多击触发有效间隔时间(ms)，为0双击事件无效
}TDL_BUTTON_CFG_T;



/***********************************************************
***********************variable define**********************
***********************************************************/
/**
* @brief button event callback function
* @param[in] name button name
* @param[in] event button trigger event
* @param[in] argc repeat count/long press time
* @return none
*/
typedef VOID (*TDL_BUTTON_EVENT_CB)(IN CHAR_T *name, IN TDL_BUTTON_TOUCH_EVENT_E event, IN VOID* argc);

/***********************************************************
***********************function define**********************
***********************************************************/

/**
* @brief Pass in the button configuration and create a button handle
* @param[in] name button name
* @param[in] button_cfg button software configuration
* @param[out] handle the handle of the control button
* @return Function Operation Result  OPRT_OK is ok other is fail 
*/
OPERATE_RET tdl_button_create(IN CHAR_T *name, IN TDL_BUTTON_CFG_T *button_cfg, OUT TDL_BUTTON_HANDLE *handle);


/**
* @brief Delete a button
* @param[in] handle the handle of the control button
* @return Function Operation Result  OPRT_OK is ok other is fail 
*/
OPERATE_RET tdl_button_delete(IN TDL_BUTTON_HANDLE handle);


/**
* @brief Function registration for button events
* @param[in] handle the handle of the control button
* @param[in] event button trigger event
* @param[in] cb The function corresponding to the button event
* @return none 
*/
VOID tdl_button_event_register(IN TDL_BUTTON_HANDLE handle, IN TDL_BUTTON_TOUCH_EVENT_E event, IN TDL_BUTTON_EVENT_CB cb);


/**
* @brief Turn button function off or on
* @param[in] enable 0-close  1-open
* @return Function Operation Result  OPRT_OK is ok other is fail 
*/
OPERATE_RET tdl_button_deep_sleep_ctrl(BOOL_T enable);

/**
* @brief set button task stack size
* 
* @param[in] size stack size
* @return Function Operation Result  OPRT_OK is ok other is fail 
*/
OPERATE_RET tdl_button_set_task_stack_size(UINT_T size);


/**
* @brief set button ready flag (sensor special use)
*		 if ready flag is false, software will filter the trigger for the first time,
*		 if use this func,please call after registered.
*        [ready flag default value is false.]
* @param[in] name button name
* @param[in] status true or false
* @return OPRT_OK if successful
*/
OPERATE_RET tdl_button_set_ready_flag(IN CHAR_T *name, IN BOOL_T status);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*_TDL_BUTTON_MANAGE_H_*/