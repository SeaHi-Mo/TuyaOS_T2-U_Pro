# tal_uart

该组件在 `tkl_uart` 的基础上进行实现，通过环形队列将串口接收到的数据缓存起来，方便上层应用进行串口数据读取。

`TAL_UART_CFG_T` 中的 `open_mode` 的部分配置需要（如 DMA） 需要下层支持才能使用。

# 使用示例
```c
    STATIC TAL_UART_CFG_T sg_uart_cfg = {
        .rx_buffer_size = MAX_DATE_LEN,
        .open_mode = O_BLOCK,
        .base_cfg = {
            .baudrate = 4800,
            .parity = TUYA_UART_PARITY_TYPE_NONE,
            .databits = TUYA_UART_DATA_LEN_8BIT,
            .stopbits = TUYA_UART_STOP_LEN_1BIT,
            .flowctrl = TUYA_UART_FLOWCTRL_NONE,
        },
    };

    tal_uart_init(TUYA_TIMER_NUM_0, &sg_uart_cfg);

    UCHAR_T send_buf[] = {0x00, 0x01, 0x02, 0x03, 0x04};
    tal_uart_write(TUYA_TIMER_NUM_0, send_buf, SIZEOF(send_buf));

    UCHAR_T recv_buf[64]={0};
    UCHAR_T recv_len = 0;
    recv_len = tal_uart_read(TUYA_TIMER_NUM_0, recv_buf, 64);
    for (UCHAR_T i=0; i<recv_len; i++) {
        TAL_PR_DEBUG("%d: %x", i, recv_buf[i]);
    }

    tal_uart_deinit(TUYA_TIMER_NUM_0);

```
