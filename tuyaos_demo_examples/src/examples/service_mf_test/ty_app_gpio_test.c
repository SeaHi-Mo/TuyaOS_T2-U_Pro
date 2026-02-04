

/**
 * @file tfm_gpio_test.c
 * @brief tfm_gpio_test module is used to
 * @version 0.1
 * @date 2022-10-31
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tkl_gpio.h"
#include "tal_log.h"
#include "ty_app_gpio_test.h"


/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/

/***********************************************************
********************function declaration********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC BOOL_T sg_enable_gpio_test = TRUE;

/***********************************************************
***********************function define**********************
***********************************************************/

VOID ty_app_gpio_test_enabe(BOOL_T enable)
{
    sg_enable_gpio_test = enable;

    return;
}

STATIC BOOL_T __ty_app_gpio_test_spcl_cb(GPIO_TEST_TABLE *table, UINT_T idx)
{
    UINT_T i, j;
    TUYA_GPIO_BASE_CFG_T gpio_cfg;

    for (i = 0; i < table->group[idx].ionum; i++) {
        // set io direction
        for (j = 0; j < table->group[idx].ionum; j++) {

            if (i == j) {
                gpio_cfg.mode = TUYA_GPIO_PUSH_PULL;
                gpio_cfg.direct = TUYA_GPIO_OUTPUT;
                gpio_cfg.level = TUYA_GPIO_LEVEL_HIGH;
                tkl_gpio_init(table->group[idx].iopin[j], &gpio_cfg);
            } else {
                gpio_cfg.mode = TUYA_GPIO_PULLUP;
                gpio_cfg.direct = TUYA_GPIO_INPUT;
                gpio_cfg.level = TUYA_GPIO_LEVEL_HIGH;
                tkl_gpio_init(table->group[idx].iopin[j], &gpio_cfg);
            }
        }
        // write 1
        TUYA_GPIO_LEVEL_E level;
        OPERATE_RET opret;

        tkl_gpio_write(table->group[idx].iopin[i], TRUE);
        for (j = 0; j < table->group[idx].ionum; j++) {
            if (i != j) {
                opret = tkl_gpio_read(table->group[idx].iopin[j], &level);
                if (opret != OPRT_OK || level != 1) {
                    TAL_PR_ERR("[%d]gpio test err_high i = %d,j = %d", idx, i, j);
                    return FALSE;
                }
            }
        }

        // write 0
        tkl_gpio_write(table->group[idx].iopin[i], FALSE);
        for (j = 0; j < table->group[idx].ionum; j++) {
            if (i != j) {
                opret = tkl_gpio_read(table->group[idx].iopin[j], &level);
                if (opret != OPRT_OK || level != 0) {
                    TAL_PR_ERR("[%d]gpio test err_high i = %d,j = %d", idx, i, j);
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

BOOL_T ty_app_gpio_test_all(IN CONST CHAR_T *in, OUT CHAR_T *out)
{
    if (!sg_enable_gpio_test) {
        return TRUE;
    }

    UCHAR_T str_len = 0;
    UCHAR_T group_cnt = 0;
    UCHAR_T io_cnt = 0;
    BOOL_T pair_flag = 0;
    CHAR_T *ptemp = NULL;
    CHAR_T *pstart = NULL;
    CHAR_T ctempbuf[16] = {0};

    if (in == NULL) {
        return FALSE;
    }

    str_len = strlen(in);
    while (str_len > 0) {
        ptemp = strstr(in, "\"G\":[");
        if (ptemp != NULL) {
            break;
        }
        ptemp++;
        str_len--;
    }

    if (str_len <= 0) { /* can't find !! */
        return FALSE;
    }

    ptemp += strlen("{\"G\":[") - 1;
    GPIO_TEST_TABLE gpio_test_item = {0};
    while (*ptemp != ']') {
        if (pair_flag) {
            if (*ptemp == ',') {
                strncpy(ctempbuf, pstart, ((ptemp - pstart) * SIZEOF(CHAR_T)));
                gpio_test_item.group[group_cnt].iopin[io_cnt] = atoi(ctempbuf);
                memset(ctempbuf, 0, SIZEOF(ctempbuf));
                pstart = ptemp + 1;
                io_cnt++;
            } else if (*ptemp == '"') {
                strncpy(ctempbuf, pstart, ((ptemp - pstart) * SIZEOF(CHAR_T)));
                gpio_test_item.group[group_cnt].iopin[io_cnt] = atoi(ctempbuf);
                memset(ctempbuf, 0, SIZEOF(ctempbuf));
                gpio_test_item.group[group_cnt].ionum = io_cnt + 1;
                pair_flag = FALSE;
                group_cnt++;
                io_cnt = 0;
            }

        } else {
            if (*ptemp == '"') {
                pair_flag = TRUE;
                pstart = ptemp + 1;
            }
        }
        ptemp++;
    }

    gpio_test_item.group_num = group_cnt;

    UINT_T idx, i;
    BOOL_T result = TRUE;
    BOOL_T ret;
    ptemp = out;
    for (idx = 0; idx < gpio_test_item.group_num; idx++) {
        ret = __ty_app_gpio_test_spcl_cb(&gpio_test_item, idx);
        if (FALSE == ret) {
            result = FALSE;

            if (strlen(out) != 0) {
                *ptemp++ = ',';
            } else {
                strcpy(ptemp, "\"G\":[");
                ptemp += strlen(out);
            }
            *ptemp++ = '\"';

            for (i = 0; i < gpio_test_item.group[idx].ionum; i++) {
                ptemp += sprintf(ptemp, "%d", gpio_test_item.group[idx].iopin[i]);
                *ptemp++ = ',';
            }
            ptemp--;
            *ptemp++ = '\"';
        }
    }

    if (FALSE == result) {
        *ptemp = ']';
    }

    return result;
}
