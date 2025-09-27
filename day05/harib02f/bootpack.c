void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int xsize, int ysize);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	extern char hankaku[4096];

	static char font_A[16] = {
		0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
		0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
	};

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	putfonts8_asc(binfo->vram, binfo->scrnx,  8, 8, COL8_FFFFFF, "Gang\'s Github.");

    for (;;) {
        io_hlt();
    }
}

void init_palette(void)
{
    static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:黑色 */
		0xff, 0x00, 0x00,	/*  1:亮红色 */
		0x00, 0xff, 0x00,	/*  2:亮绿色 */
		0xff, 0xff, 0x00,	/*  3:亮黄色 */
		0x00, 0x00, 0xff,	/*  4:亮蓝色 */
		0xff, 0x00, 0xff,	/*  5:亮紫色 */
		0x00, 0xff, 0xff,	/*  6:亮青色 */
		0xff, 0xff, 0xff,	/*  7:白色 */
		0xc6, 0xc6, 0xc6,	/*  8:亮灰色 */
		0x84, 0x00, 0x00,	/*  9:暗红色 */
		0x00, 0x84, 0x00,	/* 10:暗绿色 */
		0x84, 0x84, 0x00,	/* 11:暗黄色 */
		0x00, 0x00, 0x84,	/* 12:暗蓝色 */
		0x84, 0x00, 0x84,	/* 13:暗紫色 */
		0x00, 0x84, 0x84,	/* 14:暗青色 */
		0x84, 0x84, 0x84	/* 15:暗灰色 */
	};
    set_palette(0, 15, table_rgb);

    return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
    int i, eflags;
    eflags = io_load_eflags();
    io_cli();
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++) {
        io_out8(0x03c9, rgb[0]/4);
        io_out8(0x03c9, rgb[1]/4);
        io_out8(0x03c9, rgb[2]/4);
        rgb += 3;
    }
    io_store_eflags(eflags);
    return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
    int x, y;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++) {
            vram[y * xsize + x] = c;
        }
    }
    return;
}

void init_screen(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	boxfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	boxfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	return;
}

/**
 * @brief 从最左边（最高位）开始往最低位遍历，并设置显存中的像素颜色
 * 
 * @param vram 显存地址
 * @param xsize 屏幕宽度
 * @param x 字符显示位置的x坐标
 * @param y 字符显示位置的y坐标
 * @param c 字符颜色
 * @param font 字符字体
 */
void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d;
	for (i = 0; i < 16; i++)
	{
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) p[0] = c;
		if ((d & 0x40) != 0) p[1] = c;
		if ((d & 0x20) != 0) p[2] = c;
		if ((d & 0x10) != 0) p[3] = c;
		if ((d & 0x08) != 0) p[4] = c;
		if ((d & 0x04) != 0) p[5] = c;
		if ((d & 0x02) != 0) p[6] = c;
		if ((d & 0x01) != 0) p[7] = c;
	}
}

/**
 * @brief 显示字符串
 * 
 * @param vram 显存地址
 * @param xsize 屏幕宽度
 * @param x 字符串显示位置的x坐标
 * @param y 字符串显示位置的y坐标
 * @param c 字符串颜色
 * @param s 字符串指针
 */
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4096];
	// c语言中，字符串以0x00结尾
	for (; *s != 0; s++)
	{
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}

	return;
}