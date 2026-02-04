# 当前文件所在目录
LOCAL_PATH := $(call my-dir)

#---------------------------------------

# 清除 LOCAL_xxx 变量
include $(CLEAR_VARS)

# 当前模块名
LOCAL_MODULE := $(notdir $(LOCAL_PATH))

# 模块对外头文件（只能是目录）
# 加载至CFLAGS中提供给其他组件使用；打包进SDK产物中；
LOCAL_TUYA_SDK_INC := $(LOCAL_PATH)/include
LOCAL_TUYA_SDK_INC += $(LOCAL_PATH)/src
ifneq ($(APP_PACK_FLAG), 1) 
LOCAL_TUYA_SDK_INC += $(LOCAL_PATH)/application_components
LOCAL_TUYA_SDK_INC += $(LOCAL_PATH)/application_drivers
endif

# 模块对外CFLAGS：其他组件编译时可感知到
LOCAL_TUYA_SDK_CFLAGS := -DUSER_SW_VER=\"$(APP_VER)\" -DAPP_BIN_NAME=\"$(APP_NAME)\"

# 模块源代码
LOCAL_SRC_FILES := $(shell find $(LOCAL_PATH)/src -maxdepth 1 -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/libqrencode/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/service_ota -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/service_health_manager -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/service_http -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/service_http_download -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/service_mf_test -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/service_soc_device -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/service_time -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/system_mutex -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/system_network -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/system_queue -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/system_semaphore -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/system_sw_timer -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/system_thread -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/os_kv -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/os_uf -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/os_event -name "*.c" -o -name "*.cpp" -o -name "*.cc")

ifeq ($(CONFIG_ENABLE_ADC), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/driver_adc/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif
ifeq ($(CONFIG_ENABLE_GPIO), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/driver_gpio/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif
ifeq ($(CONFIG_ENABLE_I2C), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/driver_i2c/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif
ifeq ($(CONFIG_ENABLE_PWM), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/driver_pwm/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif
ifeq ($(CONFIG_ENABLE_SPI), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/driver_spi/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif
ifeq ($(CONFIG_ENABLE_TIMER), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/driver_timer/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif
ifeq ($(CONFIG_ENABLE_MEDIA), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/driver_speaker/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
TUYA_SDK_INC += $(LOCAL_PATH)/src/examples/driver_speaker/
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/driver_mic/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
TUYA_SDK_INC += $(LOCAL_PATH)/src/examples/driver_mic/
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/driver_dvp/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
TUYA_SDK_INC += $(LOCAL_PATH)/src/examples/driver_dvp/
endif
ifeq ($(CONFIG_ENABLE_WIFI_SERVICE), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/os_wifi/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/service_product_test -name "*.c" -o -name "*.cpp" -o -name "*.cc")
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/service_query_lowpower_dp -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif
ifeq ($(CONFIG_TUYA_WIFI_FFC_MASTER), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/service_ffc_master/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif
ifeq ($(CONFIG_TUYA_WIFI_FFC_SLAVER), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/service_ffc_slaver/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif
ifeq ($(CONFIG_ENABLE_BT_SERVICE), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/os_ble/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif
ifeq ($(CONFIG_ENABLE_BT_REMOTE_CTRL), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/service_ble_remote/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif
ifeq ($(CONFIG_ENABLE_WATCHDOG), y)
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/src/examples/os_watchdog/ -name "*.c" -o -name "*.cpp" -o -name "*.cc")
endif

# 模块内部CFLAGS：仅供本组件使用
LOCAL_CFLAGS :=

# 全局变量赋值
TUYA_SDK_INC += $(LOCAL_TUYA_SDK_INC)  # 此行勿修改
TUYA_SDK_CFLAGS += $(LOCAL_TUYA_SDK_CFLAGS)  # 此行勿修改

# 生成静态库
include $(BUILD_STATIC_LIBRARY)

# 生成动态库
include $(BUILD_SHARED_LIBRARY)

# 导出编译详情
include $(OUT_COMPILE_INFO)

#---------------------------------------

