#include <stdio.h>
#include "bootpack.h"

#define MEMMAN_FREES 4090 // 0x1000 / sizeof(struct FREEINFO)
#define MEMMAN_ADDR 0x003c0000
// 可用信息
struct FREEINFO
{
	unsigned int addr, size;
};

// 内存管理器
struct MEMMAN
{
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];

	int mx, my, i;
	struct MOUSE_DEC mdec;
	unsigned int memtotal;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

	init_gdtidt();
	init_pic();
	io_sti(); // IDT/PIC的初始化已经完成，放开CPU的中断

	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); // 11111001 PIC1以外全部禁止
	io_out8(PIC1_IMR, 0xef); // 11101111 禁止鼠标以外的中断

	init_keyboard();
	enable_mouse(&mdec);

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); // 0x00001000 - 0x0009efff
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;

	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	for (;;)
	{
		io_cli(); // 禁止中断，原因：如果在处理键盘数据时被鼠标中断打断，可能会导致数据混乱，例如：正在处理键盘数据，此时鼠标中断打断了键盘数据的处理，导致键盘数据被覆盖
		if (fifo8_status(&keyfifo) == 0 && fifo8_status(&mousefifo) == 0)
		{
			io_stihlt();
		}
		else
		{
			if (fifo8_status(&keyfifo) != 0)
			{
				i = fifo8_get(&keyfifo);

				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			}
			else if (fifo8_status(&mousefifo) != 0)
			{
				i = fifo8_get(&mousefifo);
				io_sti();

				if (mouse_decode(&mdec, i) != 0)
				{

					sprintf(s, "[lcr %4d, %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0)
					{
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0)
					{
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0)
					{
						s[2] = 'C';
					}

					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);

					// 鼠标指针移动
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* 隐藏鼠标 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0)
					{
						mx = 0;
					}
					if (my < 0)
					{
						my = 0;
					}
					if (mx > binfo->scrnx - 16)
					{
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16)
					{
						my = binfo->scrny - 16;
					}
					/* 显示鼠标 */
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);		 // 隐藏坐标
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);		 // 显示坐标
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); // 显示鼠标
				}
			}
		}
	}
}

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000
unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	// 确认CPU是386还是486以上的
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) // 如果386，即使设置了AC-bit，AC-bit也会自动变回0
	{
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	io_store_eflags(eflg);

	if (flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* 禁止缓存 */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* 允许缓存 */
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;	   // 可用信息数目
	man->maxfrees = 0; // 用于观察可用状况：frees的最大值
	man->lostsize = 0; // 释放失败的内存大小总和
	man->losts = 0;	   // 释放失败次数
}

unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++)
	{
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++)
	{
		if (man->free[i].size >= size)
		{
			// 找到足够大的内存
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0)
			{
				// 如果free[i]变成了0，就减掉一条可用信息
				man->frees--;
				for (; i < man->frees; i++)
				{
					man->free[i] = man->free[i + 1]; // 把后面的一条可用信息前移
				}
			}
			return a;
		}
	}
	return 0; // 没有可用空间
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	// 为了便于归纳内存，将free[]按照地址顺序排列
	for (i = 0; i < man->frees; i++)
	{
		if (man->free[i].addr > addr)
		{
			break;
		}
	}
	// free[i - 1].addr < addr < free[i].addr
	if (i > 0)
	{
		// 前面有可用信息
		if (man->free[i - 1].addr + man->free[i - 1].size == addr)
		{
			// 可以与前面的可用信息归纳到一起
			man->free[i - 1].size += size;
			if (i < man->frees)
			{
				// 后面也有可用信息
				if (addr + size == man->free[i].addr)
				{
					// 也可以与后面的可用信息归纳到一起
					man->free[i - 1].size += man->free[i].size;
					man->frees--;
					for (; i < man->frees; i++)
					{
						man->free[i] = man->free[i + 1]; // 把后面的一条可用信息前移
					}
				}
			}
			return 0; // 成功
		}
	}
	if (i < man->frees)
	{
		// 后面有可用信息
		if (addr + size == man->free[i].addr)
		{
			// 可以与后面的可用信息归纳到一起
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; // 成功
		}
	}
	// 既不能与前面归纳到一起，也不能与后面归纳到一起
	if (man->frees < MEMMAN_FREES)
	{
		// free[i]之后的，向后移动，腾出一点可用信息
		for (j = man->frees; j > i; j--)
		{
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; // 成功
	}
	// 不能再增加可用信息
	man->losts++;
	man->lostsize += size;
	return -1; // 失败
}