void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);

void HariMain(void)
{
    int i;
    char *p = (char *)0xa0000;

    init_palette();

    for (i = 0; i <= 0xffff; i++) {
        p[i] = i & 0x0f;
    }

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