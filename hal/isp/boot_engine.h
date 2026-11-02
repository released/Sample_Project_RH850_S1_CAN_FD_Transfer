/* Project/API reading notes: CAN update engine interface.
 * Request channel identifies the physical bus owner; data buffers and replay state belong to one
 * update session.
 * A reset action is only a request: the caller must complete the final response transmission
 * first.
 */
#ifndef RH850_BOOT_ENGINE_H
#define RH850_BOOT_ENGINE_H
#include "device_image.h"

typedef struct boot_io {
    device_read_fn read;
    unsigned char (*erase)(unsigned long address);
    unsigned char (*write_page)(unsigned long address,const unsigned char *data);
} boot_io_t;
typedef struct boot_frame {
    unsigned long id;
    unsigned char length;
    unsigned char data[64];
    unsigned char channel;
} boot_frame_t;
typedef struct boot_engine {
    boot_io_t io;
    device_image_info_t installed;
    unsigned long metadata[64],page[64],block[16],last[16];
    unsigned long next,size,block_offset,block_crc,last_offset,last_crc;
    unsigned long diagnostics;
    unsigned short staged,page_used;
    unsigned char sealed,active,ever_started,flash_ready,complete;
    unsigned char block_length,block_mask,block_received,block_bad,last_valid,last_length;
    unsigned char owner_valid,owner_channel;
} boot_engine_t;

void Boot_Init(boot_engine_t *s,const boot_io_t *io,unsigned char flash_ready);
/* Discard only RAM staging; never erase/write or reset the version policy. */
void Boot_ReleasePort(boot_engine_t *s);
/* Returns response length0/8. Raw data and BEGIN do not send an ACK.
 * action1 means END accepted; reset only after the response completed TX.
 * COMMIT acknowledges session acceptance; a physical page is written once
 * four64-byte blocks are assembled,or a final partial page is FF-padded.
 */
unsigned char Boot_Process(boot_engine_t *s,const boot_frame_t *request,
    unsigned char reply[8],unsigned char *action);

#endif
