#ifndef CAN_BOARD_H
#define CAN_BOARD_H
#include "can_config.h"

/* Hardware adapter. Replace this file's implementation when porting a board;
 * keep can_service.c and application frame processing independent of INTC/pins.
 */
rs_canfd_result_t CanBoard_Prepare(void);
rs_canfd_result_t CanBoard_Start(const rs_canfd_config_t *config);
rs_canfd_result_t CanBoard_Stop(void);
rs_canfd_result_t CanBoard_StopChannel(uint8_t channel);
rs_canfd_result_t CanBoard_StartChannel(uint8_t channel);
uint32_t CanBoard_Millis(void);
uint32_t CanBoard_Lock(void);
void CanBoard_Unlock(uint32_t saved_psw);
void CanBoard_EnableRxIrq(uint8_t enable);
void CanBoard_AckRxIrq(uint8_t repend);
uint8_t CanBoard_TakeFifoLost(uint8_t fifo);
rs_canfd_result_t CanBoard_ReadFifo(uint8_t fifo, rs_canfd_frame_t *frame);
rs_canfd_result_t CanBoard_ReadLatest(uint8_t buffer, rs_canfd_frame_t *frame);
rs_canfd_result_t CanBoard_Send(uint8_t channel, const rs_canfd_frame_t *frame);
rs_canfd_result_t CanBoard_TxResult(uint8_t channel, rs_canfd_tx_result_t *result);
rs_canfd_result_t CanBoard_Abort(uint8_t channel);
rs_canfd_result_t CanBoard_Status(uint8_t channel, rs_canfd_status_t *status);
void CanBoard_ClearErrors(uint8_t channel, uint32_t global, uint32_t errors);
#endif
