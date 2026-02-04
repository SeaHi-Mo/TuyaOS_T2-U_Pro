/**
 * @file example_http.c
 * @author www.tuya.com
 * @version 0.1
 * @date 2023-10-12
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_cloud_types.h"
#include "httpc.h"
#include "ty_cJSON.h"

#include "tal_log.h"
#include "tal_memory.h"
#include "tal_system.h"
#include "mqc_app.h"

/***********************************************************
*********************** macro define ***********************
***********************************************************/
#define API "http://httpbin.org" 
#define APIresource "/get" 

#define REQ_CONTENT               ""
#define REQ_CONTENT_LEN           SIZEOF(REQ_CONTENT)
#define READ_CONTENT_BUFF_LEN     2048u

/***********************************************************
********************** typedef define **********************
***********************************************************/


/***********************************************************
********************** variable define *********************
***********************************************************/
const http_req_t cHTTP_REQUEST_INFO = {
    .type        = HTTP_GET,
    .resource    = APIresource,
    .version     = HTTP_VER_1_1,
    .content     = REQ_CONTENT,
    .content_len = REQ_CONTENT_LEN,
};


/***********************************************************
********************** function define *********************
***********************************************************/

/**
* @brief  http task
*
* @param[in] param:Task parameters
* @return none
*/
VOID example_http(INT_T argc, CHAR_T *argv[])
{
    int http_error = WM_SUCCESS;
    http_session_t session = NULL;
    http_resp_t *response = NULL;
    http_req_t request;

    if(!get_mqc_conn_stat()) {
        TAL_PR_ERR("device not connect cloud!");
        return;
    }

    memset((UCHAR_T *)&request, 0x00, SIZEOF(http_req_t));

    http_error = http_open_session(&session, API, 0, 0);
    if (http_error != WM_SUCCESS) {
        TAL_PR_ERR("Failed to open an HTTP session err:%d", http_error);
        return;
    }

    http_error = http_prepare_req(session, &cHTTP_REQUEST_INFO, HDR_ADD_DEFAULT_USER_AGENT);
    if (http_error != WM_SUCCESS) {
        TAL_PR_ERR("Failed to prepare request infomation err:%d", http_error);
        http_close_session(&session);
        return;
    }

    http_error = http_send_request(session, &cHTTP_REQUEST_INFO, 0);
    if (http_error != WM_SUCCESS) {
        TAL_PR_ERR("Failed to send request infomation err:%d", http_error);
        http_close_session(&session);
        return;
    }

    // 获取HTTP响应头
    http_error = http_get_response_hdr(session, &response);
    if (http_error != WM_SUCCESS) {
        TAL_PR_DEBUG("Failed to get HTTP response header err:%d", http_error);
        http_close_session(&session);
        return;
    }

    // HTTP响应状态码
    if (response->status_code != HTTP_OK) {
        TAL_PR_DEBUG("HTTP request failed with status code: %d\n", response->status_code);
    }

    TAL_PR_DEBUG("reason_phrase:%s", response->reason_phrase);
    TAL_PR_DEBUG("location:%s",      response->location);
    TAL_PR_DEBUG("server:%s",        response->server);

    // 读取HTTP响应内容
    CHAR_T *p_buffer = (CHAR_T *)tal_malloc(READ_CONTENT_BUFF_LEN);
    if(NULL == p_buffer) {
        http_close_session(&session);
        return;
    }
    memset(p_buffer, 0, READ_CONTENT_BUFF_LEN);

    http_read_content(session, p_buffer, READ_CONTENT_BUFF_LEN);

    // 打印内容
    TAL_PR_INFO("rsp:%s", p_buffer);

    tal_free(p_buffer);
    http_close_session(&session);

    return;


}