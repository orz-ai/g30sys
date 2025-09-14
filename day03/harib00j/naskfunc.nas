; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 生成目标文件模式
[BITS 32]						; 生成32位模式机器码

; 目标文件信息

[FILE "naskfunc.nas"]			; 源文件名信息

		GLOBAL	_io_hlt			; 此程序中包含的函数名

; 以下是实际函数

[SECTION .text]		; 在目标文件中先写此指令再写程序

_io_hlt:	; void io_hlt(void);
		HLT
		RET