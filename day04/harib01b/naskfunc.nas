; naskfunc
; TAB=4

[FORMAT "WCOFF"]								; 生成目标文件模式
[INSTRSET "i486p"]								; 这个程序是给486用的
[BITS 32]										; 生成32位模式机器码

; 目标文件信息

[FILE "naskfunc.nas"]							; 源文件名信息

		GLOBAL	_io_hlt, _write_mem8			; 此程序中包含的函数名

; 以下是实际函数

[SECTION .text]									; 在目标文件中先写此指令再写程序

_io_hlt:	; void io_hlt(void);
		HLT
		RET

_write_mem8:	; void write_mem8(int addr, int data);
		MOV		ECX, [ESP+4]					; 将[ESP+4]中的addr的值读入ECX寄存器中
		MOV		AL, [ESP+8]						; 将[ESP+8]中的data值读入AL中
		MOV		[ECX], AL						; 将AL的值读入ECX指定的内存地址
		RET										; 返回调用者
