#ifndef _TDL_BUTTON_DRIVER_H_
#define _TDL_BUTTON_DRIVER_H_

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef VOID* DEVICE_BUTTON_HANDLE;
typedef VOID (*TDL_BUTTON_CB)(VOID *arg);

typedef enum{
	BUTTON_TIMER_SCAN_MODE = 0,
	BUTTON_IRQ_MODE,
}TDL_BUTTON_MODE_E;


typedef struct {
    DEVICE_BUTTON_HANDLE dev_handle;  //tdd handle
    TDL_BUTTON_CB   irq_cb;           //irq cb
}TDL_BUTTON_OPRT_INFO;


typedef struct{
  OPERATE_RET (*button_create)(IN TDL_BUTTON_OPRT_INFO *dev);
  OPERATE_RET (*button_delete)(IN TDL_BUTTON_OPRT_INFO *dev);
	OPERATE_RET (*read_value)(IN TDL_BUTTON_OPRT_INFO *dev, OUT UCHAR_T *value);
}TDL_BUTTON_CTRL_INFO;


typedef struct{
  VOID* dev_handle;
  TDL_BUTTON_MODE_E mode;
}TDL_BUTTON_DEVICE_INFO_T;


//按键软件配置
OPERATE_RET tdl_button_register(IN CHAR_T *name, IN TDL_BUTTON_CTRL_INFO *button_ctrl_info, TDL_BUTTON_DEVICE_INFO_T *button_cfg_info);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*_TDL_BUTTON_H_*/