#include "bootpack.h"

struct FIFO8 mousefifo;

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

/**
 * PS/2鼠标的中断处理程序
 */
void inthandler2c(int *esp)
{
    unsigned char data;
    io_out8(PIC1_OCW2, 0x64); // 通知PIC "IRQ-12已经受理完毕"
    io_out8(PIC0_OCW2, 0x62); // 通知PIC "IRQ-02已经受理完毕"
    data = io_in8(PORT_KEYDAT);

    fifo8_put(&mousefifo, data);

    return;
}

void enable_mouse(struct MOUSE_DEC *mdec)
{
    /* 激活鼠标 */
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    mdec->phase = 0; /* 顺利的话，键盘控制电路会返送ACK(0xfa) */
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
    if (mdec->phase == 0)
    {
        /* 等待鼠标的0xfa状态 */
        if (dat == 0xfa)
        {
            mdec->phase = 1;
        }
        return 0;
    }
    if (mdec->phase == 1)
    {
        /* 等待鼠标的第一字节 */
        if ((dat & 0xc8) == 0x08)
        {
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        return 0;
    }
    if (mdec->phase == 2)
    {
        /* 等待鼠标的第二字节 */
        mdec->buf[1] = dat;
        mdec->phase = 3;
        return 0;
    }
    if (mdec->phase == 3)
    {
        /* 等待鼠标的第三字节 */
        mdec->buf[2] = dat;
        mdec->phase = 1;
        mdec->btn = mdec->buf[0] & 0x07;
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];
        if ((mdec->buf[0] & 0x10) != 0)
        {
            mdec->x |= 0xffffff00;
        }
        if ((mdec->buf[0] & 0x20) != 0)
        {
            mdec->y |= 0xffffff00;
        }
        mdec->y = -mdec->y; /* 鼠标的y方向与画面符号相反 */
        return 1;
    }
    return -1; /* 应该不会到这里来 */
}
