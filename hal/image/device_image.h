#ifndef DEVICE_IMAGE_H
#define DEVICE_IMAGE_H
#include "image_metadata.h"
#include "image_metadata_config.h"

typedef unsigned char (*device_read_fn)(unsigned long address,
    unsigned char *data,unsigned short size);
typedef struct device_image_info {
    unsigned long version,build,size,version_control;
    unsigned char valid;
} device_image_info_t;
unsigned long Device_Crc32Update(unsigned long crc,const unsigned char *data,unsigned short size);
unsigned long Device_Crc32(const unsigned char *data,unsigned short size);
unsigned long Device_Read32(const unsigned char *data);
void Device_Write32(unsigned char *data,unsigned long value);
/* CRC/ranges/entry/EBASE/complete INTBP gates. Safe read is supplied by platform. */
unsigned char Device_ImageValidate(device_read_fn read,device_image_info_t *out);
unsigned char Device_ImageVectors(device_read_fn read,unsigned long size);
unsigned long Device_ImageInfo(const device_image_info_t *image,unsigned char page,
    unsigned char app,unsigned long health,unsigned long diagnostics,unsigned char *ok);

#endif
