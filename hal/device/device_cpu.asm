	.section ".text", text
	.align 4
	.public _Device_IrqSave
_Device_IrqSave:
	stsr 5,r10,0
	di
	jmp [lp]
	.public _Device_IrqRestore
_Device_IrqRestore:
	andi 0x20,r6,r10
	bnz .L.irq_kept_disabled
	ei
.L.irq_kept_disabled:
	jmp [lp]
	.public _Device_InstructionSync
_Device_InstructionSync:
	synci
	jmp [lp]
	.public _Device_ReadRomStart
_Device_ReadRomStart:
	mov #__sRAM_READ.text,r10
	jmp [lp]
	.public _Device_ReadRomSize
_Device_ReadRomSize:
	mov #__eRAM_READ.text,r10
	mov #__sRAM_READ.text,r11
	sub r11,r10
	jmp [lp]
	.public _Device_JumpAsm
_Device_JumpAsm:
	di
	mov 0x00010400,r10
	ldsr r10,4,1
	mov 0x00010200,r10
	ldsr r10,3,1
	stsr 5,r10,0
	mov 0x00008020,r11
	or r11,r10
	ldsr r10,5,0
	synci
	mov 0x00010000,r10
	jmp [r10]
