#include <stdio.h>
#include "bootpack.h"

extern struct KEYBUF keybuf;

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
	char s[40], mcursor[256];

	int mx, my, i;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC的初始化已经完成，放开CPU的中断 */

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;

	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	io_out8(PIC0_IMR, 0xf9); /* 11111001 PIC1以外全部禁止 */
	io_out8(PIC1_IMR, 0xef); /* 11101111 禁止鼠标以外的中断 */

	for (;;)
	{
		io_cli(); // 禁止中断，原因：如果在处理键盘数据时被鼠标中断打断，可能会导致数据混乱，例如：正在处理键盘数据，此时鼠标中断打断了键盘数据的处理，导致键盘数据被覆盖
		if (keybuf.next == 0)
		{
			io_stihlt();
		}
		else
		{
			i = keybuf.data[0];
			keybuf.next--;
			int j;
			for (j = 0; j < keybuf.next; j++)
			{
				keybuf.data[j] = keybuf.data[j + 1];
			}

			io_sti();
			sprintf(s, "%02X", i);
			boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
			putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
		}
	}
}
