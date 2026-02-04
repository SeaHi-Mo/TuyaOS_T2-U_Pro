/**
* @file example_iot_http.c
* @author www.tuya.com
* @version 0.1
* @date 2022-05-20
*
* @copyright Copyright (c) tuya.inc 2022
*
*/

#include "tuya_cloud_types.h"
#include "iot_httpc.h"
#include "ty_cJSON.h"

#include "tal_log.h"
#include "tal_system.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
#define WEATHER_API              "thing.weather.get"
#define API_VERSION              "1.0"

#define POST_WEATHER_REAL_TIME   "{\"codes\": [\"w.currdate\",\"w.humidity\",\"w.conditionNum\",\"w.pressure\",\"w.uvi\",\"w.windDir\",\"w.windSpeed\",\"w.sunrise\",\"w.sunset\",\"w.temp\",\"c.city\",\"c.area\",\"t.local\"]}" 
#define POST_WEATHER_FORECAST    "{\"codes\": [\"w.date.2\",\"w.conditionNum\",\"w.pressure\",\"w.uvi\",\"w.windDir\",\"w.windSpeed\",\"w.sunrise\",\"w.sunset\",\"c.city\",\"c.area\",\"t.local\"]}"

#define POST_CONTENT             POST_WEATHER_REAL_TIME    

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/

/**
* @brief  http task
*
* @param[in] param:Task parameters
* @return none
*/
VOID example_iot_http(INT_T argc, CHAR_T *argv[])
{
    OPERATE_RET rt = OPRT_OK;
    ty_cJSON* result = NULL;
    CHAR_T *print_data = NULL;

    TUYA_CALL_ERR_LOG(iot_httpc_common_post_simple(WEATHER_API, API_VERSION, POST_CONTENT, NULL, &result));
    if (NULL == result) {
        TAL_PR_DEBUG("result is NULL");
        return;
    }

    print_data = ty_cJSON_PrintUnformatted(result);
    if (NULL != print_data) {
        TAL_PR_NOTICE("%s", print_data);
        ty_cJSON_FreeBuffer(print_data);
        print_data = NULL;
    }

    ty_cJSON_Delete(result);
    result = NULL;

    return;
}
