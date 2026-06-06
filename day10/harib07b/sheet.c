#include "bootpack.h"

#define SHEET_USE 1 // 已使用

struct SHTCTL *sheetctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
    struct SHTCTL *ctl;
    int i;
    ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
    if (ctl == 0)
    {
        goto err;
    }
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = -1; // 一个sheet都没有
    for (i = 0; i < MAX_SHEETS; i++)
    {
        ctl->sheets0[i].flags = 0; // 标记为未使用
    }
    return ctl;
err:
    return 0;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
    struct SHEET *sht;
    int i;
    for (i = 0; i < MAX_SHEETS; i++)
    {
        if (ctl->sheets0[i].flags == 0)
        {
            sht = &ctl->sheets0[i];
            sht->flags = SHEET_USE; // 标记为正在使用
            sht->height = -1;       // 隐藏
            return sht;
        }
    }
    return 0; // 没有空闲的sheet
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->col_inv = col_inv;
    return;
}

void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
    int h, old = sht->height; // 存储设置前的高度信息
    // 如果指定的高度过高或过低，则进行修正
    if (height > ctl->top + 1)
    {
        height = ctl->top + 1;
    }
    if (height < -1)
    {
        height = -1;
    }
    sht->height = height; // 设置高度

    // 以下主要是进行sheets[]的重新排列
    if (old > height) // 比以前低
    {
        if (height >= 0)
        {
            // 把中间的往上提
            for (h = old; h > height; h--)
            {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }
        else
        {
            // 把上面的降下来
            for (h = old; h < ctl->top; h++)
            {
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->top--; // 因为显示中的图层减少了一个，所以最上面的图层高度下降
        }
    }
    else if (old < height) // 比以前高
    {
        if (old >= 0)
        {
            // 把中间的往下拉
            for (h = old; h < height; h++)
            {
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }
        else
        {
            // 把上面的拉上来
            for (h = height; h > ctl->top; h--)
            {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
            ctl->top++; // 因为显示中的图层增加了一个，所以最上面的图层高度提升
        }
    }
    return;
}

void sheet_refresh(struct SHTCTL *ctl)
{
    int h;
    for (h = 0; h <= ctl->top; h++)
    {
        struct SHEET *sht = ctl->sheets[h];
        int bx, by, vx, vy;
        unsigned char *buf = sht->buf;
        for (by = 0; by < sht->bysize; by++)
        {
            vy = sht->vy0 + by;
            for (bx = 0; bx < sht->bxsize; bx++)
            {
                vx = sht->vx0 + bx;
                if (buf[by * sht->bxsize + bx] != sht->col_inv)
                {
                    ctl->vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
                }
            }
        }
    }
    return;
}

void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    if (sht->height >= 0)
    {
        sheet_refresh(ctl);
    }
    return;
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
    if (sht->height >= 0)
    {
        sheet_updown(ctl, sht, -1); // 如果处于显示状态，则先设定为隐藏
    }
    sht->flags = 0; // "未使用"标志
    return;
}