; haribote-os
; TAB=4

                ORG		0xc200			; 指定程序的装载地址是内存 0xc200

                MOV     AL, 0x13        ; VGA显卡，320*320*8位彩色
                MOV     AH, 0x00
                INT     0x10
fin:
                HLT
                JMP		fin