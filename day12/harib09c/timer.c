#include "bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

struct TIMERCTL timerctl;

void init_pit(void)
{
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);

    timerctl.count = 0;   // 计数器的值
    timerctl.timeout = 0; // 超时的值
}

void inthandler20(int *esp)
{
    io_out8(PIC0_OCW2, 0x60); // 通知PIC "IRQ-00"已经受理完毕
    timerctl.count++;
    if (timerctl.timeout > 0)
    {
        timerctl.timeout--;
        if (timerctl.timeout == 0)
        {
            fifo8_put(timerctl.fifo, timerctl.data);
        }
    }
    return;
}

void settimer(unsigned int timeout, struct FIFO8 *fifo, unsigned char data)
{
    int eflags;
    eflags = io_load_eflags(); // 记录中断许可标志的值
    io_cli();                  // 由于改变timeout和data可能会在中断处理程序
    timerctl.timeout = timeout;
    timerctl.fifo = fifo;
    timerctl.data = data;
    io_store_eflags(eflags); // 恢复中断许可标志的值
}