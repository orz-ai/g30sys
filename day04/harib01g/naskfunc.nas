; naskfunc
; TAB=4

[FORMAT "WCOFF"]								; 生成目标文件模式
[INSTRSET "i486p"]								; 这个程序是给486用的
[BITS 32]										; 生成32位模式机器码

; 目标文件信息

[FILE "naskfunc.nas"]							; 源文件名信息

		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt			; 此程序中包含的函数名
		GLOBAL	_io_in8, _io_in16, _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_write_mem8

; 以下是实际函数

[SECTION .text]									; 在目标文件中先写此指令再写程序

_io_hlt:	; void io_hlt(void);
		HLT
		RET

_io_cli:	; void io_cli(void);
		CLI
		RET

_io_sti:	; void io_sti(void);
		STI
		RET

_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);
		MOV		EDX, [ESP+4]	;port
		MOV		EAX, 0
		IN		AL, DX
		RET

_io_in16:	; int io_in16(int port);
		MOV		EDX, [ESP+4]	;port
		MOV		EAX, 0
		IN		AX, DX
		RET

_io_in32:	; int io_in32(int port);
		MOV		EDX, [ESP+4]
		IN		EAX, DX
		RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX, [ESP+4]
		MOV		AL, [ESP+8]
		OUT		DX, AL
		RET

_io_out16:	; void io_out16(int port, int data);
		MOV		EDX, [ESP+4]
		MOV		EAX, [ESP+8]
		OUT		DX, AX
		RET

_io_out32: ; void io_out32(int port, int data);
		MOV		EDX, [ESP+4]
		MOV		EAX, [ESP+8]
		OUT		DX, EAX
		RET

_io_load_eflags:	;int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS 
		POP		EAX
		RET

_io_store_eflags:	; void _io_store_eflags(int eflags);
		MOV		EAX, [ESP+4]
		PUSH	EAX
		POPFD
		RET

_write_mem8:	; void write_mem8(int addr, int data);
		MOV		ECX, [ESP+4]					; 将[ESP+4]中的addr的值读入ECX寄存器中
		MOV		AL, [ESP+8]						; 将[ESP+8]中的data值读入AL中
		MOV		[ECX], AL						; 将AL的值读入ECX指定的内存地址
		RET										; 返回调用者
