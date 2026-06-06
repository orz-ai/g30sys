; haribote-os
; TAB=4

BOTPAK  EQU     0x00280000              ; bootpack的加载地址
DSKCAC  EQU     0x00100000              ; 磁盘缓存位置
DSKCAC0 EQU     0x00008000              ; 磁盘缓存位置（实模式）

; 有关BOOT_INFO
CYLS    EQU     0x0ff0                  ; 设定启动区
LEDS    EQU     0x0ff1                  ; 
VMODE   EQU     0x0ff2                  ; 关于颜色数目的信息，颜色的位数
SCRNX   EQU     0x0ff4                  ; 分辨率x(screen x)
SCRNY   EQU     0x0ff6                  ; 分辨率y(screen y)
VRAM    EQU     0x0ff8                  ; 图像缓冲区的开始地址


                ORG		0xc200			; 指定程序的装载地址是内存 0xc200

                MOV     AL, 0x13        ; VGA显卡，320*320*8位彩色
                MOV     AH, 0x00
                INT     0x10

                MOV     BYTE [VMODE], 8  ; 记录画面模式
                MOV     WORD [SCRNX], 320
                MOV     WORD [SCRNY], 200
                MOV     DWORD [VRAM], 0x000a0000

; 用BIOS取得键盘上的各种LED指示灯的状态
                MOV     AH, 0x20
                INT     0x16            ; keyboard BIOS
                MOV     [LEDS], AL

; 禁止PIC接收所有中断
;   根据AT兼容机规范，初始化PIC需要在CLI之前完成, 否则会引起挂起。PIC的初始化稍后进行
                MOV     AL, 0xff
                OUT     0x21, AL
                NOP                     ;某些机型连续OUT可能会失效,所以加入一个空操作
                OUT     0xa1, AL
                CLI                     ; 在CPU级别也禁止中断

; 设置A20GATE使用CPU能访问的1MB以上的内存
                CALL    waitkbdout
                MOV     AL, 0xd1
                OUT     0x64, AL
                CALL    waitkbdout
                MOV     AL, 0xdf
                OUT     0x60, AL
                CALL    waitkbdout

; 切换到保护模式
[INSTRSET "i486p"]
                LGDT	[GDTR0]			; 加载临时 GDT
                MOV		EAX,CR0
                AND		EAX,0x7fffffff	; 将 bit31 置 0（禁用分页）
                OR		EAX,0x00000001	; 将 bit0 置 1（切换到保护模式）
                MOV		CR0,EAX
                JMP		pipelineflush 

pipelineflush:
                MOV		AX,1*8			;  可读写 32bit 段
                MOV		DS,AX
                MOV		ES,AX
                MOV		FS,AX
                MOV		GS,AX
                MOV		SS,AX

; 传输 bootpack
                MOV		ESI,bootpack	; 源地址
                MOV		EDI,BOTPAK		; 目标地址
                MOV		ECX,512*1024/4
                CALL	memcpy

; 将磁盘数据也传输至正确位置
; 先传输引导扇区
                MOV		ESI,0x7c00		; 源地址
                MOV		EDI,DSKCAC		; 目标地址
                MOV		ECX,512/4
                CALL	memcpy

; 传输剩余全部
                MOV		ESI,DSKCAC0+512	; 源地址
                MOV		EDI,DSKCAC+512	; 目标地址
                MOV		ECX,0
                MOV		CL,BYTE [CYLS]
                IMUL	ECX,512*18*2/4	; 柱面数转换为字节数/4
                SUB		ECX,512/4		; 减去 IPL 部分
                CALL	memcpy


; asmhead 的任务已全部完成，
;	后续交由 bootpack 处理

; 启动 bootpack
		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			        ; ECX += 3;
		SHR		ECX,2			        ; ECX /= 4;
		JZ		skip			        ; 无需传输
		MOV		ESI,[EBX+20]	        ; 源地址
		ADD		ESI,EBX     
		MOV		EDI,[EBX+12]	        ; 目标地址
		CALL	memcpy  
skip:   
		MOV		ESP,[EBX+12]	        ; 栈初始值
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; AND 结果非零则跳转至 waitkbdout
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 减法结果非零则跳转至 memcpy
		RET
; 若未忘记地址大小前缀，memcpy 也可用字符串指令实现

		ALIGNB	16
GDT0:
		RESB	8				; 空选择子
		DW		0xffff,0x0000,0x9200,0x00cf	; 可读写 32bit 段
		DW		0xffff,0x0000,0x9a28,0x0047	; 可执行 32bit 段（bootpack 用）

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack: