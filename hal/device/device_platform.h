#ifndef RH850_DEVICE_PLATFORM_H
#define RH850_DEVICE_PLATFORM_H
#include "boot_engine.h"
#include "boot_app_handoff.h"

unsigned char Device_ReadPrepare(void);
unsigned char Device_FlashRead(unsigned long address,unsigned char *out,unsigned short size);
unsigned char Device_FlashInit(void);
unsigned char Device_FlashErase(unsigned long address);
unsigned char Device_FlashWritePage(unsigned long address,const unsigned char *data);
unsigned char Device_CanInit(unsigned short request_id,unsigned char raw_blocks);
unsigned char Device_CanRead(boot_frame_t *frame);
unsigned char Device_CanSend(unsigned short id,const unsigned char *data,unsigned char length);
void Device_CanStop(void);
unsigned long Device_CanDiagnostics(void);
unsigned char Device_CanLastResult(void);
unsigned char Device_CanPort(void);
unsigned long Device_Millis(void);
unsigned long Device_IrqSave(void);
void Device_IrqRestore(unsigned long psw);
void Device_InstructionSync(void);
unsigned long Device_ReadRomStart(void);
unsigned long Device_ReadRomSize(void);
void Device_JumpAsm(void);
void Device_RequestReset(unsigned long request);

#endif
