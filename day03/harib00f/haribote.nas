; haribote-os
; TAB=4

                ORG		0xc200			; 指定程序的装载地址是内存 0xc200
fin:
                HLT
                JMP		fin