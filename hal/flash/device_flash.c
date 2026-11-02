/* Project/API reading notes: MCU-specific erase/program adapter for the Boot update engine.
 * App partition and erase/page alignment checks protect Boot; library preparation and RAM
 * execution follow this MCU's Flash implementation.
 * Success means the requested library operation completed.
 */
#include "r_cg_macrodriver.h"
#include "r_fcl_types.h"
#include "r_fcl.h"
#include "fcl_descriptor.h"
#include "fcl_user.h"
#include "flash_geometry.h"
#include "device_platform.h"

#pragma section r0_disp32 "FCL_RESERVED"
unsigned char g_fcl_ram[0x7000];
#pragma section default
static unsigned char g_flash_ready;

/* FCL internal-handler mode transfers P/E execution to its copied RAM code
 * and returns only after restoring read mode. All interrupts remain masked
 * for the whole call. No UART or CAN work is reachable inside this wrapper.
 */
static unsigned char Execute(r_fcl_command_t command, unsigned long index, unsigned short count,
                             unsigned char *data)
{
    r_fcl_request_t request;
    unsigned long psw;
    request.command_enu = command;
    request.idx_u32 = index;
    request.cnt_u16 = count;
    request.bufferAdd_u32 = (uint32_t)data;
    request.status_enu = R_FCL_ERR_FLOW;
    psw = Device_IrqSave();
    FCLUser_Open();
    R_FCL_Execute(&request);
    FCLUser_Close();
    Device_IrqRestore(psw);
    return (unsigned char)(request.status_enu == R_FCL_OK);
}
/* Prepare the vendor Flash execution environment and verify device geometry before allowing an
 * update session to erase/program.
 */
unsigned char Device_FlashInit(void)
{
    unsigned long count, end, psw;
    r_fcl_status_t result;
    g_flash_ready = 0U;
    if ((unsigned long)g_fcl_ram != 0xFEDF8000UL)
    {
        return 0U;
    }
    psw = Device_IrqSave();
    result = R_FCL_Init(&sampleApp_fclConfig_enu);
    if (result == R_FCL_OK)
    {
        result = R_FCL_CopySections();
    }
    Device_IrqRestore(psw);
    if (result != R_FCL_OK || !Execute(R_FCL_CMD_PREPARE_ENV, 0UL, 0U, 0))
    {
        return 0U;
    }
    count = 0UL;
    end = 0UL;
    if (!Execute(R_FCL_CMD_GET_BLOCK_CNT, 0UL, 0U, (unsigned char *)&count) ||
        count != DEVICE_FLASH_BLOCK_COUNT ||
        !Execute(R_FCL_CMD_GET_BLOCK_END_ADDR, count - 1UL, 0U, (unsigned char *)&end) ||
        end != IMAGE_CODE_FLASH_SIZE - 1UL)
    {
        return 0U;
    }
    /* Query the first App block too: no erase is allowed on a geometry guess. */
    if (!Execute(R_FCL_CMD_GET_BLOCK_END_ADDR, 7UL, 0U, (unsigned char *)&end) ||
        end != IMAGE_APPLICATION_START - 1UL)
    {
        return 0U;
    }
    g_flash_ready = 1U;
    return 1U;
}
unsigned char Device_FlashErase(unsigned long address)
{
    unsigned long block;
    if (!g_flash_ready || !Device_FlashBlockAllowed(address, &block))
    {
        return 0U;
    }
    return Execute(R_FCL_CMD_ERASE, block, 1U, 0);
}
/* Program one validated aligned physical page through the Flash backend.
 * The transport engine must assemble/pad the full page first.
 */
unsigned char Device_FlashWritePage(unsigned long address, const unsigned char *data)
{
    if (!g_flash_ready || !data || ((unsigned long)data & 3UL) || !Device_FlashPageAllowed(address))
    {
        return 0U;
    }
    return Execute(R_FCL_CMD_WRITE, address, 1U, (unsigned char *)data);
}
