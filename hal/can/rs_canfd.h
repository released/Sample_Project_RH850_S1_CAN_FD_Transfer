#ifndef RS_CANFD_H
#define RS_CANFD_H

#include "rs_canfd_regs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum e_rs_canfd_result
    {
        RS_CANFD_OK = 0,
        RS_CANFD_ERR_PARAM,
        RS_CANFD_ERR_STATE,
        RS_CANFD_ERR_TIMEOUT,
        RS_CANFD_ERR_BUSY,
        RS_CANFD_ERR_EMPTY,
        RS_CANFD_ERR_FULL
    } rs_canfd_result_t;

    typedef enum e_rs_canfd_id_format
    {
        RS_CANFD_ID_STANDARD = 0,
        RS_CANFD_ID_EXTENDED = 1
    } rs_canfd_id_format_t;

    typedef enum e_rs_canfd_frame_type
    {
        RS_CANFD_FRAME_DATA = 0,
        RS_CANFD_FRAME_REMOTE = 1
    } rs_canfd_frame_type_t;

    typedef enum e_rs_canfd_frame_mode
    {
        RS_CANFD_FRAME_CLASSIC = 0,
        RS_CANFD_FRAME_FD = 1
    } rs_canfd_frame_mode_t;

    typedef enum e_rs_canfd_channel_fd_mode
    {
        RS_CANFD_CHANNEL_MIXED = 0,
        RS_CANFD_CHANNEL_FD_ONLY = 1,
        RS_CANFD_CHANNEL_CLASSIC_ONLY = 2
    } rs_canfd_channel_fd_mode_t;

    typedef enum e_rs_canfd_global_mode
    {
        RS_CANFD_GLOBAL_MODE_OPERATING = 0,
        RS_CANFD_GLOBAL_MODE_RESET = 1,
        RS_CANFD_GLOBAL_MODE_TEST = 2,
        RS_CANFD_GLOBAL_MODE_STOP = 3
    } rs_canfd_global_mode_t;

    typedef enum e_rs_canfd_channel_mode
    {
        RS_CANFD_CHANNEL_MODE_COMMUNICATION = 0,
        RS_CANFD_CHANNEL_MODE_RESET = 1,
        RS_CANFD_CHANNEL_MODE_HALT = 2,
        RS_CANFD_CHANNEL_MODE_STOP = 3
    } rs_canfd_channel_mode_t;

    typedef enum e_rs_canfd_payload_size
    {
        RS_CANFD_PAYLOAD_8_BYTES = 0,
        RS_CANFD_PAYLOAD_12_BYTES = 1,
        RS_CANFD_PAYLOAD_16_BYTES = 2,
        RS_CANFD_PAYLOAD_20_BYTES = 3,
        RS_CANFD_PAYLOAD_24_BYTES = 4,
        RS_CANFD_PAYLOAD_32_BYTES = 5,
        RS_CANFD_PAYLOAD_48_BYTES = 6,
        RS_CANFD_PAYLOAD_64_BYTES = 7
    } rs_canfd_payload_size_t;

    typedef enum e_rs_canfd_fifo_depth
    {
        RS_CANFD_FIFO_DEPTH_0 = 0,
        RS_CANFD_FIFO_DEPTH_4 = 1,
        RS_CANFD_FIFO_DEPTH_8 = 2,
        RS_CANFD_FIFO_DEPTH_16 = 3,
        RS_CANFD_FIFO_DEPTH_32 = 4,
        RS_CANFD_FIFO_DEPTH_48 = 5,
        RS_CANFD_FIFO_DEPTH_64 = 6,
        RS_CANFD_FIFO_DEPTH_128 = 7
    } rs_canfd_fifo_depth_t;

    typedef enum e_rs_canfd_fifo_interrupt_timing
    {
        RS_CANFD_FIFO_INT_1_8 = 0,
        RS_CANFD_FIFO_INT_2_8 = 1,
        RS_CANFD_FIFO_INT_3_8 = 2,
        RS_CANFD_FIFO_INT_4_8 = 3,
        RS_CANFD_FIFO_INT_5_8 = 4,
        RS_CANFD_FIFO_INT_6_8 = 5,
        RS_CANFD_FIFO_INT_7_8 = 6,
        RS_CANFD_FIFO_INT_FULL = 7
    } rs_canfd_fifo_interrupt_timing_t;

    typedef enum e_rs_canfd_txrx_fifo_mode
    {
        RS_CANFD_TXRX_FIFO_MODE_RECEIVE = 0,
        RS_CANFD_TXRX_FIFO_MODE_TRANSMIT = 1,
        RS_CANFD_TXRX_FIFO_MODE_GATEWAY = 2
    } rs_canfd_txrx_fifo_mode_t;

    typedef enum e_rs_canfd_test_mode
    {
        RS_CANFD_TEST_MODE_INTERNAL_LOOPBACK = 0,
        RS_CANFD_TEST_MODE_EXTERNAL_LOOPBACK = 1,
        RS_CANFD_TEST_MODE_LISTEN_ONLY = 2
    } rs_canfd_test_mode_t;

/*
 * User configuration helpers.
 *
 * Normal application code should prefer these named constants/macros and the
 * receive filter builder API below. They describe CAN behavior in application
 * terms, then the driver maps them to Section 24 register fields.
 */
#define RS_CANFD_DISABLE (0U)
#define RS_CANFD_ENABLE (1U)
#define RS_CANFD_LABEL_NONE (0x0000U)
#define RS_CANFD_MINIMUM_DLC_ANY (0U)
#define RS_CANFD_STANDARD_ID_MASK_EXACT (0x07FFU)
#define RS_CANFD_EXTENDED_ID_MASK_EXACT (0x1FFFFFFFUL)
#define RS_CANFD_ID_MASK_ACCEPT_ALL (0x00000000UL)
#define RS_CANFD_TX_DELAY_COMPENSATION_OFF (0U)
#define RS_CANFD_TX_DELAY_COMPENSATION_OFFSET_0 (0U)

#define RS_CANFD_ARRAY_COUNT(array) ((uint8_t)(sizeof(array) / sizeof((array)[0])))

/* RX ownership is selected per channel in can_user_config.h. Explicit
 * low-level polling/interrupt presets remain available below; the service
 * never combines polling and ISR reads for the same FIFO.
 */

/*
 * Current board timing preset:
 * - fCAN = 40 MHz
 * - nominal phase = 500 kbps
 * - data phase = 2 Mbps
 *
 * Use this macro in rs_canfd_channel_config_t.bit_timing initializers. If a new
 * baud rate is required, add a new timing preset next to this one with a name
 * that states the clock and bit rates.
 */
#define RS_CANFD_BIT_TIMING_40MHZ_N500K_D2M                                                        \
    {                                                                                              \
        0U,  /* NBRP: 40 MHz / ((0 + 1) * 80 tq) = 500 kbps. */                                    \
        58U, /* NTSEG1: 59 tq. */                                                                  \
        19U, /* NTSEG2: 20 tq. */                                                                  \
        19U, /* NSJW: 20 tq. */                                                                    \
        0U,  /* DBRP: 40 MHz / ((0 + 1) * 20 tq) = 2 Mbps. */                                      \
        13U, /* DTSEG1: 14 tq. */                                                                  \
        4U,  /* DTSEG2: 5 tq. */                                                                   \
        4U   /* DSJW: 5 tq. */                                                                     \
    }

    typedef struct st_rs_canfd_bit_timing
    {
        uint16_t nominal_brp;  /* RCFDCnCFDCmNCFG.NBRP encoded value. */
        uint8_t nominal_tseg1; /* RCFDCnCFDCmNCFG.NTSEG1 encoded value. */
        uint8_t nominal_tseg2; /* RCFDCnCFDCmNCFG.NTSEG2 encoded value. */
        uint8_t nominal_sjw;   /* RCFDCnCFDCmNCFG.NSJW encoded value. */
        uint8_t data_brp;      /* RCFDCnCFDCmDCFG.DBRP encoded value. */
        uint8_t data_tseg1;    /* RCFDCnCFDCmDCFG.DTSEG1 encoded value. */
        uint8_t data_tseg2;    /* RCFDCnCFDCmDCFG.DTSEG2 encoded value. */
        uint8_t data_sjw;      /* RCFDCnCFDCmDCFG.DSJW encoded value. */
    } rs_canfd_bit_timing_t;

    typedef struct st_rs_canfd_frame
    {
        uint32_t id;                      /* TMID/RMID/RFID/CFID[28:0]. */
        rs_canfd_id_format_t id_format;   /* TMIDE/RMIDE/RFIDE/CFIDE. */
        rs_canfd_frame_type_t frame_type; /* TMRTR/RMRTR/RFRTR/CFRTR. */
        rs_canfd_frame_mode_t frame_mode; /* TMFDF/RMFDF/RFFDF/CFFDF. */
        uint8_t brs;                      /* TMBRS/RMBRS/RFBRS/CFBRS. */
        uint8_t esi;                      /* TMESI/RMESI/RFESI/CFESI. */
        uint8_t store_history;            /* THLEN. */
        uint8_t dlc;                      /* TMDLC/RMDLC/RFDLC/CFDLC. */
        uint8_t length;                   /* Data bytes used by software. */
        uint16_t label;                   /* TMPTR/RMPTR/RFPTR/CFPTR label. */
        uint16_t timestamp;               /* RMPTR/RFPTR/CFPTR timestamp. */
        uint8_t data[RS_CANFD_FRAME_MAX_BYTES];
    } rs_canfd_frame_t;

    typedef enum e_rs_canfd_receive_id_match
    {
        RS_CANFD_RECEIVE_ID_ANY = 0,
        RS_CANFD_RECEIVE_ID_STANDARD = 1,
        RS_CANFD_RECEIVE_ID_EXTENDED = 2
    } rs_canfd_receive_id_match_t;

    typedef enum e_rs_canfd_receive_frame_match
    {
        RS_CANFD_RECEIVE_FRAME_ANY = 0,
        RS_CANFD_RECEIVE_FRAME_DATA = 1,
        RS_CANFD_RECEIVE_FRAME_REMOTE = 2
    } rs_canfd_receive_frame_match_t;

    typedef struct st_rs_canfd_receive_filter
    {
        rs_canfd_receive_id_match_t id_match;       /* Any, standard ID, or extended ID. */
        uint32_t id;                                /* ID value to accept. */
        uint32_t id_mask;                           /* 1 bits are compared; 0 bits are ignored. */
        rs_canfd_receive_frame_match_t frame_match; /* Data, remote, or either frame type. */
        uint8_t minimum_dlc;        /* Reject frames with DLC lower than this value. */
        uint8_t accept_own_tx;      /* Accept frames transmitted by this RS-CANFD unit. */
        uint8_t route_to_rx_buffer; /* Route matching frames to an RX buffer. */
        uint8_t rx_buffer_index;    /* RX buffer index used when route_to_rx_buffer is 1. */
        uint8_t route_to_rx_fifo;   /* Route matching frames to an RX FIFO. */
        uint8_t rx_fifo;            /* RX FIFO index used when route_to_rx_fifo is 1. */
        uint8_t route_to_txrx_fifo; /* Route matching frames to a TX/RX FIFO. */
        uint8_t txrx_fifo;          /* TX/RX FIFO index used when route_to_txrx_fifo is 1. */
        uint16_t label;             /* Software label copied to received frame metadata. */
    } rs_canfd_receive_filter_t;

/*
 * Static receive filter initializers.
 *
 * These are intended for config tables. They create one receive rule table
 * entry without exposing GAFL bit layout to the user.
 *
 * ACCEPT_ALL disables ID and IDE comparison, so both standard and extended IDs
 * can pass. STANDARD_ID and EXTENDED_ID compare only the bits selected by
 * id_mask: 1 = compare this ID bit, 0 = ignore this ID bit.
 *
 * The *_RX_FIFO_AND_BUFFER variants route the same accepted frame to an RX FIFO
 * and to one receive buffer. The FIFO is the queued receive path; the receive
 * buffer is a latest-frame shadow that is useful for bring-up diagnostics and
 * matches the proven sample-project receive path.
 */
#define RS_CANFD_RECEIVE_FILTER_ACCEPT_ALL_TO_RX_FIFO(rx_fifo_number)                              \
    {RS_CANFD_RECEIVE_ID_ANY,                                                                      \
     0x00000000UL,                                                                                 \
     RS_CANFD_ID_MASK_ACCEPT_ALL,                                                                  \
     RS_CANFD_RECEIVE_FRAME_ANY,                                                                   \
     RS_CANFD_MINIMUM_DLC_ANY,                                                                     \
     RS_CANFD_DISABLE,                                                                             \
     RS_CANFD_DISABLE,                                                                             \
     0U,                                                                                           \
     RS_CANFD_ENABLE,                                                                              \
     (rx_fifo_number),                                                                             \
     RS_CANFD_DISABLE,                                                                             \
     0U,                                                                                           \
     RS_CANFD_LABEL_NONE}

#define RS_CANFD_RECEIVE_FILTER_ACCEPT_ALL_TO_RX_FIFO_AND_BUFFER(rx_fifo_number, rx_buffer_number) \
    {RS_CANFD_RECEIVE_ID_ANY,                                                                      \
     0x00000000UL,                                                                                 \
     RS_CANFD_ID_MASK_ACCEPT_ALL,                                                                  \
     RS_CANFD_RECEIVE_FRAME_ANY,                                                                   \
     RS_CANFD_MINIMUM_DLC_ANY,                                                                     \
     RS_CANFD_DISABLE,                                                                             \
     RS_CANFD_ENABLE,                                                                              \
     (rx_buffer_number),                                                                           \
     RS_CANFD_ENABLE,                                                                              \
     (rx_fifo_number),                                                                             \
     RS_CANFD_DISABLE,                                                                             \
     0U,                                                                                           \
     RS_CANFD_LABEL_NONE}

#define RS_CANFD_RECEIVE_FILTER_STANDARD_ID_TO_RX_FIFO(id_value, id_mask_value, rx_fifo_number)    \
    {RS_CANFD_RECEIVE_ID_STANDARD,                                                                 \
     ((uint32_t)(id_value) & 0x07FFUL),                                                            \
     ((uint32_t)(id_mask_value) & 0x07FFUL),                                                       \
     RS_CANFD_RECEIVE_FRAME_ANY,                                                                   \
     RS_CANFD_MINIMUM_DLC_ANY,                                                                     \
     RS_CANFD_DISABLE,                                                                             \
     RS_CANFD_DISABLE,                                                                             \
     0U,                                                                                           \
     RS_CANFD_ENABLE,                                                                              \
     (rx_fifo_number),                                                                             \
     RS_CANFD_DISABLE,                                                                             \
     0U,                                                                                           \
     RS_CANFD_LABEL_NONE}

#define RS_CANFD_RECEIVE_FILTER_STANDARD_ID_TO_RX_FIFO_AND_BUFFER(                                 \
    id_value, id_mask_value, rx_fifo_number, rx_buffer_number)                                     \
    {RS_CANFD_RECEIVE_ID_STANDARD,                                                                 \
     ((uint32_t)(id_value) & 0x07FFUL),                                                            \
     ((uint32_t)(id_mask_value) & 0x07FFUL),                                                       \
     RS_CANFD_RECEIVE_FRAME_ANY,                                                                   \
     RS_CANFD_MINIMUM_DLC_ANY,                                                                     \
     RS_CANFD_DISABLE,                                                                             \
     RS_CANFD_ENABLE,                                                                              \
     (rx_buffer_number),                                                                           \
     RS_CANFD_ENABLE,                                                                              \
     (rx_fifo_number),                                                                             \
     RS_CANFD_DISABLE,                                                                             \
     0U,                                                                                           \
     RS_CANFD_LABEL_NONE}

#define RS_CANFD_RECEIVE_FILTER_EXTENDED_ID_TO_RX_FIFO(id_value, id_mask_value, rx_fifo_number)    \
    {RS_CANFD_RECEIVE_ID_EXTENDED,                                                                 \
     ((uint32_t)(id_value) & 0x1FFFFFFFUL),                                                        \
     ((uint32_t)(id_mask_value) & 0x1FFFFFFFUL),                                                   \
     RS_CANFD_RECEIVE_FRAME_ANY,                                                                   \
     RS_CANFD_MINIMUM_DLC_ANY,                                                                     \
     RS_CANFD_DISABLE,                                                                             \
     RS_CANFD_DISABLE,                                                                             \
     0U,                                                                                           \
     RS_CANFD_ENABLE,                                                                              \
     (rx_fifo_number),                                                                             \
     RS_CANFD_DISABLE,                                                                             \
     0U,                                                                                           \
     RS_CANFD_LABEL_NONE}

#define RS_CANFD_RECEIVE_FILTER_EXTENDED_ID_TO_RX_FIFO_AND_BUFFER(                                 \
    id_value, id_mask_value, rx_fifo_number, rx_buffer_number)                                     \
    {RS_CANFD_RECEIVE_ID_EXTENDED,                                                                 \
     ((uint32_t)(id_value) & 0x1FFFFFFFUL),                                                        \
     ((uint32_t)(id_mask_value) & 0x1FFFFFFFUL),                                                   \
     RS_CANFD_RECEIVE_FRAME_ANY,                                                                   \
     RS_CANFD_MINIMUM_DLC_ANY,                                                                     \
     RS_CANFD_DISABLE,                                                                             \
     RS_CANFD_ENABLE,                                                                              \
     (rx_buffer_number),                                                                           \
     RS_CANFD_ENABLE,                                                                              \
     (rx_fifo_number),                                                                             \
     RS_CANFD_DISABLE,                                                                             \
     0U,                                                                                           \
     RS_CANFD_LABEL_NONE}

    /*
     * Advanced hardware receive rule.
     *
     * Normal application code should use rs_canfd_receive_filter_t plus the helper
     * functions below. This raw form exists so driver code can map directly to
     * RCFDCnCFDGAFLIDj/Mj/P0_j/P1_j names in Section 24.
     */
    typedef struct st_rs_canfd_rx_rule
    {
        uint32_t id;                  /* RCFDCnCFDGAFLIDj.GAFLID. */
        uint32_t id_mask;             /* RCFDCnCFDGAFLMj.GAFLIDM. */
        uint8_t ide;                  /* RCFDCnCFDGAFLIDj.GAFLIDE. */
        uint8_t ide_compare;          /* RCFDCnCFDGAFLMj.GAFLIDEM. */
        uint8_t rtr;                  /* RCFDCnCFDGAFLIDj.GAFLRTR. */
        uint8_t rtr_compare;          /* RCFDCnCFDGAFLMj.GAFLRTRM. */
        uint8_t local_loopback;       /* RCFDCnCFDGAFLIDj.GAFLLB. */
        uint8_t minimum_dlc;          /* RCFDCnCFDGAFLP0_j.GAFLDLC. */
        uint8_t use_receive_buffer;   /* RCFDCnCFDGAFLP0_j.GAFLRMV. */
        uint8_t receive_buffer_index; /* RCFDCnCFDGAFLP0_j.GAFLRMDP. */
        uint16_t label;               /* RCFDCnCFDGAFLP0_j.GAFLPTR. */
        uint8_t rx_fifo_mask;         /* RCFDCnCFDGAFLP1_j.GAFLFDP[7:0]. */
        uint32_t txrx_fifo_mask;      /* RCFDCnCFDGAFLP1_j.GAFLFDP[25:8]. */
    } rs_canfd_rx_rule_t;

    typedef struct st_rs_canfd_global_config
    {
        uint8_t use_canfd_clock;           /* RCFDCnCFDGCFG.DCS: 0=fCAN from clkc. */
        uint8_t mirror_mode_enable;        /* RCFDCnCFDGCFG.MME. */
        uint8_t dlc_check_enable;          /* RCFDCnCFDGCFG.DCE. */
        uint8_t dlc_replace_enable;        /* RCFDCnCFDGCFG.DRE. */
        uint8_t tx_priority_by_id;         /* RCFDCnCFDGCFG.TPRI. */
        uint8_t timestamp_source;          /* RCFDCnCFDGCFG.TSSS. */
        uint8_t timestamp_prescaler;       /* RCFDCnCFDGCFG.TSP. */
        uint8_t timestamp_clock_source;    /* RCFDCnCFDGCFG.TSBTCS. */
        uint16_t interval_timer_prescaler; /* RCFDCnCFDGCFG.ITRCP. */
    } rs_canfd_global_config_t;

    typedef struct st_rs_canfd_rx_buffer_config
    {
        uint8_t count;                   /* RCFDCnCFDRMNB.NRXMB. */
        rs_canfd_payload_size_t payload; /* RCFDCnCFDRMNB.RMPLS. */
    } rs_canfd_rx_buffer_config_t;

    typedef struct st_rs_canfd_rx_fifo_config
    {
        uint8_t fifo;                     /* RCFDCnCFDRFCCx index. */
        uint8_t enable;                   /* RCFDCnCFDRFCCx.RFE. */
        uint8_t interrupt_enable;         /* RCFDCnCFDRFCCx.RFIE. */
        uint8_t interrupt_at_every_frame; /* RCFDCnCFDRFCCx.RFIM. */
        rs_canfd_fifo_interrupt_timing_t interrupt_timing;
        rs_canfd_fifo_depth_t depth;     /* RCFDCnCFDRFCCx.RFDC. */
        rs_canfd_payload_size_t payload; /* RCFDCnCFDRFCCx.RFPLS. */
    } rs_canfd_rx_fifo_config_t;

    typedef struct st_rs_canfd_txrx_fifo_config
    {
        uint8_t fifo;                     /* RCFDCnCFDCFCCk index. */
        uint8_t enable;                   /* RCFDCnCFDCFCCk.CFE. */
        uint8_t rx_interrupt_enable;      /* RCFDCnCFDCFCCk.CFRXIE. */
        uint8_t tx_interrupt_enable;      /* RCFDCnCFDCFCCk.CFTXIE. */
        uint8_t interrupt_at_every_frame; /* RCFDCnCFDCFCCk.CFIM. */
        rs_canfd_fifo_interrupt_timing_t interrupt_timing;
        rs_canfd_fifo_depth_t depth;     /* RCFDCnCFDCFCCk.CFDC. */
        rs_canfd_payload_size_t payload; /* RCFDCnCFDCFCCk.CFPLS. */
        rs_canfd_txrx_fifo_mode_t mode;  /* RCFDCnCFDCFCCk.CFM. */
        uint8_t linked_tx_buffer;        /* RCFDCnCFDCFCCk.CFTML. */
        uint8_t interval_timer_enable;   /* RCFDCnCFDCFCCk.CFITR. */
        uint8_t interval_timer_source;   /* RCFDCnCFDCFCCk.CFITSS. */
        uint8_t interval_timer;          /* RCFDCnCFDCFCCk.CFITT. */
    } rs_canfd_txrx_fifo_config_t;

    typedef struct st_rs_canfd_tx_queue_config
    {
        uint8_t enable;                   /* RCFDCnCFDTXQCCm.TXQE. */
        uint8_t interrupt_enable;         /* RCFDCnCFDTXQCCm.TXQTXIE. */
        uint8_t interrupt_at_every_frame; /* RCFDCnCFDTXQCCm.TXQIM. */
        uint8_t depth;                    /* RCFDCnCFDTXQCCm.TXQDC. */
    } rs_canfd_tx_queue_config_t;

    typedef struct st_rs_canfd_tx_history_config
    {
        uint8_t enable;                   /* RCFDCnCFDTHLCCm.THLE. */
        uint8_t interrupt_enable;         /* RCFDCnCFDTHLCCm.THLIE. */
        uint8_t interrupt_at_every_entry; /* RCFDCnCFDTHLCCm.THLIM. */
        uint8_t store_data;               /* RCFDCnCFDTHLCCm.THLDTE. */
        uint8_t interrupt_threshold;      /* RCFDCnCFDTHLCCm.THLIMC. */
    } rs_canfd_tx_history_config_t;

    typedef struct st_rs_canfd_interrupt_config
    {
        uint8_t global_error_enable;       /* RCFDCnCFDGCTR.MEIE/DEIE. */
        uint8_t channel_error_enable;      /* RCFDCnCFDCmCTR error interrupt bits. */
        uint8_t tx_complete_enable;        /* RCFDCnCFDCmCTR.TAIE. */
        uint32_t tx_buffer_interrupt_mask; /* RCFDCnCFDTMIECm.TMIE. */
    } rs_canfd_interrupt_config_t;

    typedef struct st_rs_canfd_dma_trigger_config
    {
        uint8_t rx_fifo_mask;   /* RCFDCnCFDCDTCT.RFDMAE[7:0]. */
        uint8_t txrx_fifo_mask; /* RCFDCnCFDCDTCT.CFDMAE[7:0]. */
    } rs_canfd_dma_trigger_config_t;

    typedef struct st_rs_canfd_channel_config
    {
        uint8_t channel; /* RCFDCnCFDCm*. */
        uint8_t enable;
        rs_canfd_channel_fd_mode_t fd_mode;
        rs_canfd_bit_timing_t bit_timing;
        uint8_t tx_delay_compensation_enable;
        uint8_t tx_delay_compensation_offset;
        const rs_canfd_receive_filter_t
            *receive_filters;         /* High-level GAFL receive rule table entries. */
        uint8_t receive_filter_count; /* RCFDCnCFDGAFLCFGx.RNC value for this channel. */
        const rs_canfd_rx_fifo_config_t *rx_fifos;
        uint8_t rx_fifo_count;
        const rs_canfd_txrx_fifo_config_t *txrx_fifos;
        uint8_t txrx_fifo_count;
        rs_canfd_tx_queue_config_t tx_queue;
        rs_canfd_tx_history_config_t tx_history;
        rs_canfd_interrupt_config_t interrupts;
    } rs_canfd_channel_config_t;

/*
 * Static buffer/channel initializers.
 *
 * These macros are for rs_canfd_config_t tables. A user adding a channel should
 * be able to read the row as "enable mixed CAN FD, use this timing preset, use
 * these receive filters, and enable these RX FIFOs".
 */
#define RS_CANFD_RX_BUFFER_CONFIG_DISABLED {0U, RS_CANFD_PAYLOAD_64_BYTES}

#define RS_CANFD_RX_BUFFER_CONFIG_64_BYTES(rx_buffer_count)                                        \
    {(rx_buffer_count), RS_CANFD_PAYLOAD_64_BYTES}

#define RS_CANFD_RX_FIFO_CONFIG_64_BYTES(rx_fifo_number, fifo_depth_value)                         \
    {(rx_fifo_number),       RS_CANFD_ENABLE,    RS_CANFD_ENABLE,          RS_CANFD_ENABLE,        \
     RS_CANFD_FIFO_INT_FULL, (fifo_depth_value), RS_CANFD_PAYLOAD_64_BYTES}

/*
 * Polling-oriented RX FIFO preset.
 *
 * This matches the proven v2 sample behavior for bring-up:
 * - RCFDCnCFDRFCCx.RFIE = 0: no module-side RX FIFO interrupt request.
 * - RCFDCnCFDRFCCx.RFIM = 1 and RFIGCV = FULL: interrupt generation compare
 *   value is harmless when RFIE is disabled, but the register value stays close
 *   to the v2 reference dump.
 * - RCFDCnCFDRFCCx.RFPLS = 64 bytes and RFDC uses fifo_depth_value.
 */
#define RS_CANFD_RX_FIFO_CONFIG_64_BYTES_POLLING(rx_fifo_number, fifo_depth_value)                 \
    {(rx_fifo_number),       RS_CANFD_ENABLE,    RS_CANFD_DISABLE,         RS_CANFD_ENABLE,        \
     RS_CANFD_FIFO_INT_FULL, (fifo_depth_value), RS_CANFD_PAYLOAD_64_BYTES}

/*
 * Interrupt-oriented RX FIFO preset.
 *
 * This keeps the v2-compatible threshold setting, but enables the module-side
 * RX FIFO interrupt request:
 * - RCFDCnCFDRFCCx.RFIE = 1.
 * - RCFDCnCFDRFCCx.RFIM = 1 and RFIGCV = FULL.
 * - Expected enabled value with depth 16 / 64-byte payload: 0x0000F373.
 */
#define RS_CANFD_RX_FIFO_CONFIG_64_BYTES_INTERRUPT(rx_fifo_number, fifo_depth_value)               \
    {(rx_fifo_number),       RS_CANFD_ENABLE,    RS_CANFD_ENABLE,          RS_CANFD_ENABLE,        \
     RS_CANFD_FIFO_INT_FULL, (fifo_depth_value), RS_CANFD_PAYLOAD_64_BYTES}

#define RS_CANFD_TX_QUEUE_CONFIG_DISABLED {RS_CANFD_DISABLE, RS_CANFD_DISABLE, RS_CANFD_DISABLE, 0U}

#define RS_CANFD_TX_HISTORY_CONFIG_DISABLED                                                        \
    {RS_CANFD_DISABLE, RS_CANFD_DISABLE, RS_CANFD_DISABLE, RS_CANFD_DISABLE, 0U}

#define RS_CANFD_INTERRUPT_CONFIG_DISABLED                                                         \
    {RS_CANFD_DISABLE, RS_CANFD_DISABLE, RS_CANFD_DISABLE, 0x00000000UL}

#define RS_CANFD_CHANNEL_CONFIG_MIXED_RX_FIFO(channel_number, timing_preset, filter_array,         \
                                              rx_fifo_array)                                       \
    {(channel_number),                                                                             \
     RS_CANFD_ENABLE,                                                                              \
     RS_CANFD_CHANNEL_MIXED,                                                                       \
     timing_preset,                                                                                \
     RS_CANFD_TX_DELAY_COMPENSATION_OFF,                                                           \
     RS_CANFD_TX_DELAY_COMPENSATION_OFFSET_0,                                                      \
     (filter_array),                                                                               \
     RS_CANFD_ARRAY_COUNT(filter_array),                                                           \
     (rx_fifo_array),                                                                              \
     RS_CANFD_ARRAY_COUNT(rx_fifo_array),                                                          \
     0,                                                                                            \
     0U,                                                                                           \
     RS_CANFD_TX_QUEUE_CONFIG_DISABLED,                                                            \
     RS_CANFD_TX_HISTORY_CONFIG_DISABLED,                                                          \
     RS_CANFD_INTERRUPT_CONFIG_DISABLED}

    typedef struct st_rs_canfd_config
    {
        volatile rs_canfd_unit_t *unit; /* RCFDC0 register base. */
        rs_canfd_global_config_t global;
        rs_canfd_rx_buffer_config_t rx_buffer; /* One shared pool, configured once. */
        const rs_canfd_channel_config_t *channels;
        uint8_t channel_count;
    } rs_canfd_config_t;

    typedef struct st_rs_canfd_status
    {
        uint32_t global_status;       /* RCFDCnCFDGSTS. */
        uint32_t global_error_flags;  /* RCFDCnCFDGERFL. */
        uint32_t channel_status;      /* RCFDCnCFDCmSTS. */
        uint32_t channel_error_flags; /* RCFDCnCFDCmERFL. */
        uint32_t rx_fifo_status[RS_CANFD_RX_FIFO_COUNT];
        uint32_t txrx_fifo_status[RS_CANFD_TXRX_FIFO_COUNT];
        uint32_t tx_queue_status;
        uint32_t tx_history_status;
    } rs_canfd_status_t;

    typedef struct st_rs_canfd_tx_history_entry
    {
        uint8_t valid;
        uint8_t tx_buffer_index;
        uint16_t label;
        uint16_t timestamp;
        uint32_t raw_acc0;
        uint32_t raw_acc1;
    } rs_canfd_tx_history_entry_t;

    typedef enum e_rs_canfd_config_issue
    {
        RS_CANFD_CONFIG_VALID = 0,
        RS_CANFD_CONFIG_NULL,
        RS_CANFD_CONFIG_CHANNEL,
        RS_CANFD_CONFIG_TIMING,
        RS_CANFD_CONFIG_FIFO,
        RS_CANFD_CONFIG_FILTER,
        RS_CANFD_CONFIG_BUFFER,
        RS_CANFD_CONFIG_RAM
    } rs_canfd_config_issue_t;

    typedef struct st_rs_canfd_config_report
    {
        rs_canfd_config_issue_t issue;
        uint8_t channel;
        uint8_t index;
        uint32_t message_ram_bytes;
        uint16_t rule_count;
    } rs_canfd_config_report_t;

    typedef enum e_rs_canfd_tx_result
    {
        RS_CANFD_TX_NONE = 0,
        RS_CANFD_TX_PENDING,
        RS_CANFD_TX_COMPLETE,
        RS_CANFD_TX_ABORTED
    } rs_canfd_tx_result_t;

    /* Validate before touching registers. Report channel/index identifies the
     * offending config row/resource; message_ram_bytes excludes fixed TX RAM.
     */
    rs_canfd_result_t rs_canfd_validate_config(const rs_canfd_config_t *config,
                                               rs_canfd_config_report_t *report);
    rs_canfd_result_t rs_canfd_validate_frame(const rs_canfd_frame_t *frame);
    void rs_canfd_frame_clear(rs_canfd_frame_t *frame);

    /* Optional monotonic millisecond source. Init still has a finite guard if a
     * supplied timer stops. Set before initialization; do not change while active.
     */
    typedef uint32_t (*rs_canfd_millis_t)(void);
    void rs_canfd_set_time_source(rs_canfd_millis_t source, uint32_t timeout_ms);
    rs_canfd_result_t rs_canfd_get_tx_result(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                             uint8_t tx_buffer_index, rs_canfd_tx_result_t *result);
    rs_canfd_result_t rs_canfd_request_abort(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                             uint8_t tx_buffer_index);

    uint8_t rs_canfd_dlc_to_length(uint8_t dlc);
    uint8_t rs_canfd_length_to_dlc(uint8_t length);

    /*
     * Receive filter builder API for the Section 24 receive rule table.
     *
     * These helpers describe what the application wants to receive and where the
     * message should be routed. The driver converts this description to the GAFL
     * receive rule table registers during rs_canfd_init(); users do not need to
     * compose RCFDCnCFDGAFLIDj/Mj/P0_j/P1_j bits directly.
     */
    void rs_canfd_receive_filter_clear(rs_canfd_receive_filter_t *filter);
    rs_canfd_result_t
    rs_canfd_receive_filter_accept_any_to_rx_fifo(rs_canfd_receive_filter_t *filter, uint8_t fifo);
    rs_canfd_result_t
    rs_canfd_receive_filter_accept_any_to_rx_fifo_and_buffer(rs_canfd_receive_filter_t *filter,
                                                             uint8_t fifo, uint8_t buffer);
    rs_canfd_result_t rs_canfd_receive_filter_accept_standard_id_to_rx_fifo(
        rs_canfd_receive_filter_t *filter, uint16_t id, uint16_t id_mask, uint8_t fifo);
    rs_canfd_result_t rs_canfd_receive_filter_accept_standard_id_to_rx_fifo_and_buffer(
        rs_canfd_receive_filter_t *filter, uint16_t id, uint16_t id_mask, uint8_t fifo,
        uint8_t buffer);
    rs_canfd_result_t rs_canfd_receive_filter_accept_extended_id_to_rx_fifo(
        rs_canfd_receive_filter_t *filter, uint32_t id, uint32_t id_mask, uint8_t fifo);
    rs_canfd_result_t rs_canfd_receive_filter_accept_extended_id_to_rx_fifo_and_buffer(
        rs_canfd_receive_filter_t *filter, uint32_t id, uint32_t id_mask, uint8_t fifo,
        uint8_t buffer);
    void rs_canfd_receive_filter_accept_all(rs_canfd_receive_filter_t *filter);
    void rs_canfd_receive_filter_accept_standard_id(rs_canfd_receive_filter_t *filter, uint16_t id,
                                                    uint16_t id_mask);
    void rs_canfd_receive_filter_accept_extended_id(rs_canfd_receive_filter_t *filter, uint32_t id,
                                                    uint32_t id_mask);
    void rs_canfd_receive_filter_accept_data_frames(rs_canfd_receive_filter_t *filter);
    void rs_canfd_receive_filter_accept_remote_frames(rs_canfd_receive_filter_t *filter);
    void rs_canfd_receive_filter_accept_data_or_remote(rs_canfd_receive_filter_t *filter);
    void rs_canfd_receive_filter_set_minimum_dlc(rs_canfd_receive_filter_t *filter,
                                                 uint8_t minimum_dlc);
    void rs_canfd_receive_filter_accept_own_tx(rs_canfd_receive_filter_t *filter, uint8_t enable);
    rs_canfd_result_t rs_canfd_receive_filter_route_to_rx_fifo(rs_canfd_receive_filter_t *filter,
                                                               uint8_t fifo);
    rs_canfd_result_t rs_canfd_receive_filter_route_to_rx_buffer(rs_canfd_receive_filter_t *filter,
                                                                 uint8_t buffer);
    rs_canfd_result_t rs_canfd_receive_filter_route_to_txrx_fifo(rs_canfd_receive_filter_t *filter,
                                                                 uint8_t fifo);
    void rs_canfd_receive_filter_set_label(rs_canfd_receive_filter_t *filter, uint16_t label);

    rs_canfd_result_t rs_canfd_init(const rs_canfd_config_t *config);
    rs_canfd_result_t rs_canfd_deinit(volatile rs_canfd_unit_t *unit);
    rs_canfd_result_t rs_canfd_enter_global_mode(volatile rs_canfd_unit_t *unit,
                                                 rs_canfd_global_mode_t mode);
    rs_canfd_result_t rs_canfd_enter_channel_mode(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                                  rs_canfd_channel_mode_t mode);

    rs_canfd_result_t rs_canfd_configure_receive_rules(volatile rs_canfd_unit_t *unit,
                                                       uint8_t channel,
                                                       const rs_canfd_rx_rule_t *rules,
                                                       uint8_t rule_count, uint8_t page);
    rs_canfd_result_t rs_canfd_configure_receive_filters(volatile rs_canfd_unit_t *unit,
                                                         uint8_t channel,
                                                         const rs_canfd_receive_filter_t *filters,
                                                         uint8_t filter_count);
    rs_canfd_result_t rs_canfd_configure_buffers(volatile rs_canfd_unit_t *unit,
                                                 const rs_canfd_channel_config_t *channel_config);
    rs_canfd_result_t
    rs_canfd_configure_interrupts(volatile rs_canfd_unit_t *unit,
                                  const rs_canfd_channel_config_t *channel_config);

    rs_canfd_result_t rs_canfd_send_tx_buffer(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                              uint8_t tx_buffer_index,
                                              const rs_canfd_frame_t *frame);
    rs_canfd_result_t rs_canfd_abort_tx_buffer(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                               uint8_t tx_buffer_index);
    rs_canfd_result_t rs_canfd_write_txrx_fifo(volatile rs_canfd_unit_t *unit, uint8_t fifo,
                                               const rs_canfd_frame_t *frame);
    rs_canfd_result_t rs_canfd_send_tx_queue(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                             const rs_canfd_frame_t *frame);

    rs_canfd_result_t rs_canfd_read_rx_buffer(volatile rs_canfd_unit_t *unit,
                                              uint8_t rx_buffer_index, rs_canfd_frame_t *frame);
    rs_canfd_result_t rs_canfd_read_rx_fifo(volatile rs_canfd_unit_t *unit, uint8_t fifo,
                                            rs_canfd_frame_t *frame);
    rs_canfd_result_t rs_canfd_read_txrx_fifo(volatile rs_canfd_unit_t *unit, uint8_t fifo,
                                              rs_canfd_frame_t *frame);
    rs_canfd_result_t rs_canfd_read_tx_history(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                               rs_canfd_tx_history_entry_t *entry);

    rs_canfd_result_t rs_canfd_get_global_status(volatile rs_canfd_unit_t *unit,
                                                 rs_canfd_status_t *status);
    rs_canfd_result_t rs_canfd_get_channel_status(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                                  rs_canfd_status_t *status);
    rs_canfd_result_t rs_canfd_clear_error_flags(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                                 uint32_t global_mask, uint32_t channel_mask);
    rs_canfd_result_t rs_canfd_configure_dma_trigger(volatile rs_canfd_unit_t *unit,
                                                     const rs_canfd_dma_trigger_config_t *config);
    rs_canfd_result_t rs_canfd_enter_self_test(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                               rs_canfd_test_mode_t mode);
    rs_canfd_result_t rs_canfd_run_ram_test(volatile rs_canfd_unit_t *unit);

#ifdef __cplusplus
}
#endif

#endif
