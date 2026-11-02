#ifndef RS_CANFD_REGS_H
#define RS_CANFD_REGS_H

#include "rs_canfd_platform.h"
#include <stddef.h>

/*
 * RH850/F1KM-S1 RS-CANFD register layer.
 *
 * Names intentionally follow Section 24 "CANFD Interface (RS-CANFD)" so that
 * source references can be searched directly in the hardware manual.
 */

#define RS_CANFD_RX_FIFO_COUNT (8U)
#define RS_CANFD_TXRX_FIFO_COUNT (3U * RS_CANFD_MAX_CHANNELS)
#define RS_CANFD_TX_QUEUE_COUNT RS_CANFD_MAX_CHANNELS
#define RS_CANFD_TX_HISTORY_COUNT RS_CANFD_MAX_CHANNELS
#define RS_CANFD_RX_RULE_PAGE_SIZE (16U)
#define RS_CANFD_MAX_RX_RULES (128U) /* Bounded software rule budget. */
#define RS_CANFD_RX_BUFFER_COUNT (16U * RS_CANFD_MAX_CHANNELS)
#define RS_CANFD_TX_BUFFER_COUNT (32U * RS_CANFD_MAX_CHANNELS)
#define RS_CANFD_CHANNEL_TX_BUFFER_COUNT (32U)
#define RS_CANFD_FRAME_MAX_BYTES (64U)
#define RS_CANFD_RAM_TEST_PAGE_COUNT (512U)
#define RS_CANFD_RAM_TEST_PAGE_WORDS (64U)
#define RS_CANFD_WAIT_TIMEOUT (0x00FFFFFFUL)
/* F1KM target profile; check this profile when changing MCU.
 * Hardware manual 24.10.1.5: 3584 bytes per implemented channel.
 * This is the RX buffer/FIFO pool, not CPU RAM or the fixed TX buffer area.
 */
#define RS_CANFD_MESSAGE_RAM_BYTES (3584UL * RS_CANFD_MAX_CHANNELS)

#define RS_CANFD_BIT(pos) ((uint32_t)(1UL << (pos)))
#define RS_CANFD_FIELD_MASK(pos, width) ((uint32_t)(((1UL << (width)) - 1UL) << (pos)))
#define RS_CANFD_FIELD_PREP(pos, width, val)                                                       \
    ((uint32_t)(((uint32_t)(val) & ((1UL << (width)) - 1UL)) << (pos)))
#define RS_CANFD_FIELD_GET(reg, pos, width)                                                        \
    ((uint32_t)(((uint32_t)(reg) >> (pos)) & ((1UL << (width)) - 1UL)))

/* RCFDCnCFDGCFG - Global Configuration Register, Section 24.6.2. */
#define RS_CANFD_CFDGCFG_TPRI RS_CANFD_BIT(0)
#define RS_CANFD_CFDGCFG_DCE RS_CANFD_BIT(1)
#define RS_CANFD_CFDGCFG_DRE RS_CANFD_BIT(2)
#define RS_CANFD_CFDGCFG_MME RS_CANFD_BIT(3)
#define RS_CANFD_CFDGCFG_DCS RS_CANFD_BIT(4)
#define RS_CANFD_CFDGCFG_CMPOC RS_CANFD_BIT(5)
#define RS_CANFD_CFDGCFG_TSP_POS (8U)
#define RS_CANFD_CFDGCFG_TSP_WIDTH (4U)
#define RS_CANFD_CFDGCFG_TSSS RS_CANFD_BIT(12)
#define RS_CANFD_CFDGCFG_TSBTCS_POS (13U)
#define RS_CANFD_CFDGCFG_TSBTCS_WIDTH (3U)
#define RS_CANFD_CFDGCFG_ITRCP_POS (16U)
#define RS_CANFD_CFDGCFG_ITRCP_WIDTH (16U)

/* RCFDCnCFDGCTR - Global Control Register, Section 24.6.1. */
#define RS_CANFD_CFDGCTR_GMDC_POS (0U)
#define RS_CANFD_CFDGCTR_GMDC_WIDTH (2U)
#define RS_CANFD_CFDGCTR_GSLPR RS_CANFD_BIT(2)
#define RS_CANFD_CFDGCTR_DEIE RS_CANFD_BIT(8)
#define RS_CANFD_CFDGCTR_MEIE RS_CANFD_BIT(9)
#define RS_CANFD_CFDGCTR_THLEIE RS_CANFD_BIT(10)
#define RS_CANFD_CFDGCTR_CMPOFIE RS_CANFD_BIT(11)
#define RS_CANFD_CFDGCTR_TSRST RS_CANFD_BIT(16)

/* RCFDCnCFDGSTS - Global Status Register, Section 24.6.3. */
#define RS_CANFD_CFDGSTS_GRSTSTS RS_CANFD_BIT(0)
#define RS_CANFD_CFDGSTS_GHLTSTS RS_CANFD_BIT(1)
#define RS_CANFD_CFDGSTS_GSLPSTS RS_CANFD_BIT(2)
#define RS_CANFD_CFDGSTS_GRAMINIT RS_CANFD_BIT(3)

/* RCFDCnCFDCmNCFG - Channel Nominal Bit Rate Configuration Register. */
#define RS_CANFD_CFDCNCFG_NTSEG2_POS (24U)
#define RS_CANFD_CFDCNCFG_NTSEG2_WIDTH (5U)
#define RS_CANFD_CFDCNCFG_NTSEG1_POS (16U)
#define RS_CANFD_CFDCNCFG_NTSEG1_WIDTH (7U)
#define RS_CANFD_CFDCNCFG_NSJW_POS (11U)
#define RS_CANFD_CFDCNCFG_NSJW_WIDTH (5U)
#define RS_CANFD_CFDCNCFG_NBRP_POS (0U)
#define RS_CANFD_CFDCNCFG_NBRP_WIDTH (10U)

/* RCFDCnCFDCmDCFG - Channel Data Bit Rate Configuration Register. */
#define RS_CANFD_CFDCDCFG_DSJW_POS (24U)
#define RS_CANFD_CFDCDCFG_DSJW_WIDTH (3U)
#define RS_CANFD_CFDCDCFG_DTSEG2_POS (20U)
#define RS_CANFD_CFDCDCFG_DTSEG2_WIDTH (3U)
#define RS_CANFD_CFDCDCFG_DTSEG1_POS (16U)
#define RS_CANFD_CFDCDCFG_DTSEG1_WIDTH (4U)
#define RS_CANFD_CFDCDCFG_DBRP_POS (0U)
#define RS_CANFD_CFDCDCFG_DBRP_WIDTH (8U)

/* RCFDCnCFDCmFDCFG - Channel CAN FD Configuration Register. */
#define RS_CANFD_CFDCFDCFG_TDCOC RS_CANFD_BIT(8)
#define RS_CANFD_CFDCFDCFG_TDCE RS_CANFD_BIT(9)
#define RS_CANFD_CFDCFDCFG_TDCO_POS (16U)
#define RS_CANFD_CFDCFDCFG_TDCO_WIDTH (7U)
#define RS_CANFD_CFDCFDCFG_FDOE RS_CANFD_BIT(28)
#define RS_CANFD_CFDCFDCFG_REFE RS_CANFD_BIT(29)
#define RS_CANFD_CFDCFDCFG_CLOE RS_CANFD_BIT(30)

/* RCFDCnCFDCmCTR - Channel Control Register, Section 24.6.8. */
#define RS_CANFD_CFDCCCTR_CHMDC_POS (0U)
#define RS_CANFD_CFDCCCTR_CHMDC_WIDTH (2U)
#define RS_CANFD_CFDCCCTR_CSLPR RS_CANFD_BIT(2)
#define RS_CANFD_CFDCCCTR_RTBO RS_CANFD_BIT(3)
#define RS_CANFD_CFDCCCTR_BEIE RS_CANFD_BIT(8)
#define RS_CANFD_CFDCCCTR_EWIE RS_CANFD_BIT(9)
#define RS_CANFD_CFDCCCTR_EPIE RS_CANFD_BIT(10)
#define RS_CANFD_CFDCCCTR_BOEIE RS_CANFD_BIT(11)
#define RS_CANFD_CFDCCCTR_BORIE RS_CANFD_BIT(12)
#define RS_CANFD_CFDCCCTR_OLIE RS_CANFD_BIT(13)
#define RS_CANFD_CFDCCCTR_BLIE RS_CANFD_BIT(14)
#define RS_CANFD_CFDCCCTR_ALIE RS_CANFD_BIT(15)
#define RS_CANFD_CFDCCCTR_TAIE RS_CANFD_BIT(16)
#define RS_CANFD_CFDCCCTR_EOCOIE RS_CANFD_BIT(17)
#define RS_CANFD_CFDCCCTR_SOCOIE RS_CANFD_BIT(18)
#define RS_CANFD_CFDCCCTR_TDCVFIE RS_CANFD_BIT(19)
#define RS_CANFD_CFDCCCTR_CTME RS_CANFD_BIT(24)
#define RS_CANFD_CFDCCCTR_CTMS_POS (25U)
#define RS_CANFD_CFDCCCTR_CTMS_WIDTH (2U)
#define RS_CANFD_CFDCCCTR_CRCT RS_CANFD_BIT(30)
#define RS_CANFD_CFDCCCTR_ROM RS_CANFD_BIT(31)

/* RCFDCnCFDCmSTS - Channel Status Register, Section 24.6.10. */
#define RS_CANFD_CFDCCSTS_CRSTSTS RS_CANFD_BIT(0)
#define RS_CANFD_CFDCCSTS_CHLTSTS RS_CANFD_BIT(1)
#define RS_CANFD_CFDCCSTS_CSLPSTS RS_CANFD_BIT(2)
#define RS_CANFD_CFDCCSTS_BOSTS RS_CANFD_BIT(3)
#define RS_CANFD_CFDCCSTS_EPSTS RS_CANFD_BIT(4)
#define RS_CANFD_CFDCCSTS_REC_POS (8U)
#define RS_CANFD_CFDCCSTS_REC_WIDTH (8U)
#define RS_CANFD_CFDCCSTS_TEC_POS (16U)
#define RS_CANFD_CFDCCSTS_TEC_WIDTH (8U)

/* RCFDCnCFDGAFLECTR / RCFDCnCFDGAFLCFGx - Receive Rule Registers. */
#define RS_CANFD_CFDGAFLECTR_AFLPN_POS (0U)
#define RS_CANFD_CFDGAFLECTR_AFLPN_WIDTH (6U)
#define RS_CANFD_CFDGAFLECTR_AFLDAE RS_CANFD_BIT(8)
#define RS_CANFD_CFDGAFLCFG_RNC_WIDTH (8U)

/* RCFDCnCFDGAFLIDj/Mj/P0_j/P1_j - Receive Rule Entry Registers. */
#define RS_CANFD_CFDGAFLID_GAFLID_POS (0U)
#define RS_CANFD_CFDGAFLID_GAFLID_WIDTH (29U)
#define RS_CANFD_CFDGAFLID_GAFLLB RS_CANFD_BIT(29)
#define RS_CANFD_CFDGAFLID_GAFLRTR RS_CANFD_BIT(30)
#define RS_CANFD_CFDGAFLID_GAFLIDE RS_CANFD_BIT(31)
#define RS_CANFD_CFDGAFLM_GAFLIDM_POS (0U)
#define RS_CANFD_CFDGAFLM_GAFLIDM_WIDTH (29U)
#define RS_CANFD_CFDGAFLM_GAFLRTRM RS_CANFD_BIT(30)
#define RS_CANFD_CFDGAFLM_GAFLIDEM RS_CANFD_BIT(31)
#define RS_CANFD_CFDGAFLP0_GAFLDLC_POS (0U)
#define RS_CANFD_CFDGAFLP0_GAFLDLC_WIDTH (4U)
#define RS_CANFD_CFDGAFLP0_GAFLRMDP_POS (8U)
#define RS_CANFD_CFDGAFLP0_GAFLRMDP_WIDTH (7U)
#define RS_CANFD_CFDGAFLP0_GAFLRMV RS_CANFD_BIT(15)
#define RS_CANFD_CFDGAFLP0_GAFLPTR_POS (16U)
#define RS_CANFD_CFDGAFLP0_GAFLPTR_WIDTH (16U)
#define RS_CANFD_CFDGAFLP1_GAFLFDP_RX_POS (0U)
#define RS_CANFD_CFDGAFLP1_GAFLFDP_RX_WIDTH (8U)
#define RS_CANFD_CFDGAFLP1_GAFLFDP_CF_POS (8U)
#define RS_CANFD_CFDGAFLP1_GAFLFDP_CF_WIDTH (18U)

/* RCFDCnCFDRMNB/CFDRMNDy - Receive Buffer Configuration/Status. */
#define RS_CANFD_CFDRMNB_NRXMB_POS (0U)
#define RS_CANFD_CFDRMNB_NRXMB_WIDTH (8U)
#define RS_CANFD_CFDRMNB_RMPLS_POS (8U)
#define RS_CANFD_CFDRMNB_RMPLS_WIDTH (3U)
#define RS_CANFD_CFDRMND_RMNS_WIDTH (32U)

/* RCFDCnCFDRFCCx/CFDRFSTSx/CFDRFPCTRx - Receive FIFO Registers. */
#define RS_CANFD_CFDRFCC_RFE RS_CANFD_BIT(0)
#define RS_CANFD_CFDRFCC_RFIE RS_CANFD_BIT(1)
#define RS_CANFD_CFDRFCC_RFPLS_POS (4U)
#define RS_CANFD_CFDRFCC_RFPLS_WIDTH (3U)
#define RS_CANFD_CFDRFCC_RFDC_POS (8U)
#define RS_CANFD_CFDRFCC_RFDC_WIDTH (3U)
#define RS_CANFD_CFDRFCC_RFIM RS_CANFD_BIT(12)
#define RS_CANFD_CFDRFCC_RFIGCV_POS (13U)
#define RS_CANFD_CFDRFCC_RFIGCV_WIDTH (3U)
#define RS_CANFD_CFDRFSTS_RFEMP RS_CANFD_BIT(0)
#define RS_CANFD_CFDRFSTS_RFFLL RS_CANFD_BIT(1)
#define RS_CANFD_CFDRFSTS_RFMLT RS_CANFD_BIT(2)
#define RS_CANFD_CFDRFSTS_RFIF RS_CANFD_BIT(3)
#define RS_CANFD_CFDRFSTS_RFMC_POS (8U)
#define RS_CANFD_CFDRFSTS_RFMC_WIDTH (8U)
#define RS_CANFD_CFDRFPCTR_RFPC (0x000000FFUL)

/* RCFDCnCFDCFCCk/CFDCFSTSk/CFDCFPCTRk - TX/RX FIFO Registers. */
#define RS_CANFD_CFDCFCC_CFE RS_CANFD_BIT(0)
#define RS_CANFD_CFDCFCC_CFRXIE RS_CANFD_BIT(1)
#define RS_CANFD_CFDCFCC_CFTXIE RS_CANFD_BIT(2)
#define RS_CANFD_CFDCFCC_CFPLS_POS (4U)
#define RS_CANFD_CFDCFCC_CFPLS_WIDTH (3U)
#define RS_CANFD_CFDCFCC_CFM_POS (8U)
#define RS_CANFD_CFDCFCC_CFM_WIDTH (2U)
#define RS_CANFD_CFDCFCC_CFITSS RS_CANFD_BIT(10)
#define RS_CANFD_CFDCFCC_CFITR RS_CANFD_BIT(11)
#define RS_CANFD_CFDCFCC_CFIM RS_CANFD_BIT(12)
#define RS_CANFD_CFDCFCC_CFIGCV_POS (13U)
#define RS_CANFD_CFDCFCC_CFIGCV_WIDTH (3U)
#define RS_CANFD_CFDCFCC_CFTML_POS (16U)
#define RS_CANFD_CFDCFCC_CFTML_WIDTH (5U)
#define RS_CANFD_CFDCFCC_CFDC_POS (21U)
#define RS_CANFD_CFDCFCC_CFDC_WIDTH (3U)
#define RS_CANFD_CFDCFCC_CFITT_POS (24U)
#define RS_CANFD_CFDCFCC_CFITT_WIDTH (8U)
#define RS_CANFD_CFDCFSTS_CFEMP RS_CANFD_BIT(0)
#define RS_CANFD_CFDCFSTS_CFFLL RS_CANFD_BIT(1)
#define RS_CANFD_CFDCFSTS_CFMLT RS_CANFD_BIT(2)
#define RS_CANFD_CFDCFSTS_CFRXIF RS_CANFD_BIT(3)
#define RS_CANFD_CFDCFSTS_CFTXIF RS_CANFD_BIT(4)
#define RS_CANFD_CFDCFSTS_CFMC_POS (8U)
#define RS_CANFD_CFDCFSTS_CFMC_WIDTH (8U)
#define RS_CANFD_CFDCFPCTR_CFPC (0x000000FFUL)

/* RCFDCnCFDTMCp/CFDTMSTSp/CFDTMIDp/CFDTMPTRp/CFDTMFDCTRp - TX Buffer. */
#define RS_CANFD_CFDTMC_TMTR RS_CANFD_BIT(0)
#define RS_CANFD_CFDTMC_TMTAR RS_CANFD_BIT(1)
#define RS_CANFD_CFDTMC_TMOM RS_CANFD_BIT(2)
#define RS_CANFD_CFDTMSTS_TMTSTS RS_CANFD_BIT(0)
#define RS_CANFD_CFDTMSTS_TMTRF_POS (1U)
#define RS_CANFD_CFDTMSTS_TMTRF_WIDTH (2U)
#define RS_CANFD_CFDTMSTS_TMTRM RS_CANFD_BIT(3)
#define RS_CANFD_CFDTMSTS_TMTARM RS_CANFD_BIT(4)
#define RS_CANFD_CFDTMID_TMID_POS (0U)
#define RS_CANFD_CFDTMID_TMID_WIDTH (29U)
#define RS_CANFD_CFDTMID_THLEN RS_CANFD_BIT(29)
#define RS_CANFD_CFDTMID_TMRTR RS_CANFD_BIT(30)
#define RS_CANFD_CFDTMID_TMIDE RS_CANFD_BIT(31)
#define RS_CANFD_CFDTMPTR_TMPTR_POS (0U)
#define RS_CANFD_CFDTMPTR_TMPTR_WIDTH (16U)
#define RS_CANFD_CFDTMPTR_TMDLC_POS (28U)
#define RS_CANFD_CFDTMPTR_TMDLC_WIDTH (4U)
#define RS_CANFD_CFDTMFDCTR_TMESI RS_CANFD_BIT(0)
#define RS_CANFD_CFDTMFDCTR_TMBRS RS_CANFD_BIT(1)
#define RS_CANFD_CFDTMFDCTR_TMFDF RS_CANFD_BIT(2)

/* RCFDCnCFDRMIDq/CFDRMPTRq/CFDRMFDSTSq and FIFO access register fields. */
#define RS_CANFD_CFD_MSG_ID_POS (0U)
#define RS_CANFD_CFD_MSG_ID_WIDTH (29U)
#define RS_CANFD_CFD_MSG_RTR RS_CANFD_BIT(30)
#define RS_CANFD_CFD_MSG_IDE RS_CANFD_BIT(31)
#define RS_CANFD_CFD_MSG_PTR_LABEL_POS (0U)
#define RS_CANFD_CFD_MSG_PTR_LABEL_WIDTH (16U)
#define RS_CANFD_CFD_MSG_PTR_TS_POS (0U)
#define RS_CANFD_CFD_MSG_PTR_TS_WIDTH (16U)
#define RS_CANFD_CFD_MSG_PTR_DLC_POS (28U)
#define RS_CANFD_CFD_MSG_PTR_DLC_WIDTH (4U)
#define RS_CANFD_CFD_MSG_FDSTS_ESI RS_CANFD_BIT(0)
#define RS_CANFD_CFD_MSG_FDSTS_BRS RS_CANFD_BIT(1)
#define RS_CANFD_CFD_MSG_FDSTS_FDF RS_CANFD_BIT(2)
#define RS_CANFD_CFD_MSG_FDSTS_PTR_POS (16U)
#define RS_CANFD_CFD_MSG_FDSTS_PTR_WIDTH (16U)

/* RCFDCnCFDTXQCCm/CFDTXQSTSm/CFDTXQPCTRm - TX Queue Registers. */
#define RS_CANFD_CFDTXQCC_TXQE RS_CANFD_BIT(0)
#define RS_CANFD_CFDTXQCC_TXQTXIE RS_CANFD_BIT(5)
#define RS_CANFD_CFDTXQCC_TXQIM RS_CANFD_BIT(7)
#define RS_CANFD_CFDTXQCC_TXQDC_POS (8U)
#define RS_CANFD_CFDTXQCC_TXQDC_WIDTH (5U)
#define RS_CANFD_CFDTXQSTS_TXQEMP RS_CANFD_BIT(0)
#define RS_CANFD_CFDTXQSTS_TXQFLL RS_CANFD_BIT(1)
#define RS_CANFD_CFDTXQSTS_TXQTXIF RS_CANFD_BIT(2)
#define RS_CANFD_CFDTXQPCTR_TXQPC (0x000000FFUL)

/* RCFDCnCFDTHLCCm/CFDTHLSTSm/CFDTHLPCTRm - TX History List Registers. */
#define RS_CANFD_CFDTHLCC_THLE RS_CANFD_BIT(0)
#define RS_CANFD_CFDTHLCC_THLIE RS_CANFD_BIT(1)
#define RS_CANFD_CFDTHLCC_THLIM RS_CANFD_BIT(2)
#define RS_CANFD_CFDTHLCC_THLDTE RS_CANFD_BIT(3)
#define RS_CANFD_CFDTHLCC_THLIMC_POS (8U)
#define RS_CANFD_CFDTHLCC_THLIMC_WIDTH (8U)
#define RS_CANFD_CFDTHLSTS_THLELT RS_CANFD_BIT(0)
#define RS_CANFD_CFDTHLSTS_THLFLL RS_CANFD_BIT(1)
#define RS_CANFD_CFDTHLSTS_THLIF RS_CANFD_BIT(2)
#define RS_CANFD_CFDTHLPCTR_THLPC (0x000000FFUL)

/* RCFDCnCFDCDTCT/CFDCDTSTS - CAN DMA Trigger Registers, Section 24.6.49. */
#define RS_CANFD_CFDCDT_RFDMAE_POS (0U)
#define RS_CANFD_CFDCDT_RFDMAE_WIDTH (8U)
#define RS_CANFD_CFDCDT_CFDMAE_POS (8U)
#define RS_CANFD_CFDCDT_CFDMAE_WIDTH (8U)

/* RCFDCnCFDGTSTCTR - Global Test Control Register, Section 24.10. */
#define RS_CANFD_CFDGTSTCFG_RTMPS_POS (0U)
#define RS_CANFD_CFDGTSTCFG_RTMPS_WIDTH (9U)
#define RS_CANFD_CFDGTSTCTR_ICBCTME RS_CANFD_BIT(0)
#define RS_CANFD_CFDGTSTCTR_RTME RS_CANFD_BIT(2)
#define RS_CANFD_CFDGLOCKK_RAM_TEST_KEY1 (0x7575U)
#define RS_CANFD_CFDGLOCKK_RAM_TEST_KEY2 (0x8A8AU)

typedef struct st_rs_canfd_channel_regs
{
    rs_canfd_reg32_t ncfg; /* RCFDCnCFDCmNCFG */
    rs_canfd_reg32_t ctr;  /* RCFDCnCFDCmCTR */
    rs_canfd_reg32_t sts;  /* RCFDCnCFDCmSTS */
    rs_canfd_reg32_t erfl; /* RCFDCnCFDCmERFL */
} rs_canfd_channel_regs_t;

typedef struct st_rs_canfd_channel_fd_regs
{
    rs_canfd_reg32_t dcfg;  /* RCFDCnCFDCmDCFG */
    rs_canfd_reg32_t fdcfg; /* RCFDCnCFDCmFDCFG */
    rs_canfd_reg32_t fdctr; /* RCFDCnCFDCmFDCTR */
    rs_canfd_reg32_t fdsts; /* RCFDCnCFDCmFDSTS */
    rs_canfd_reg32_t fdcrc; /* RCFDCnCFDCmFDCRC */
    unsigned char reserved[12];
} rs_canfd_channel_fd_regs_t;

typedef struct st_rs_canfd_rx_rule_regs
{
    rs_canfd_reg32_t id;   /* RCFDCnCFDGAFLIDj */
    rs_canfd_reg32_t mask; /* RCFDCnCFDGAFLMj */
    rs_canfd_reg32_t ptr0; /* RCFDCnCFDGAFLP0_j */
    rs_canfd_reg32_t ptr1; /* RCFDCnCFDGAFLP1_j */
} rs_canfd_rx_rule_regs_t;

typedef struct st_rs_canfd_message_ram_regs
{
    rs_canfd_reg32_t id;       /* RCFDCnCFDRMIDq / CFDRFIDx / CFDCFIDk */
    rs_canfd_reg32_t ptr;      /* RCFDCnCFDRMPTRq / CFDRFPTRx / CFDCFPTRk */
    rs_canfd_reg32_t fdsts;    /* RCFDCnCFDRMFDSTSq / CFDRFFDSTSx / CFDCFFDCSTSk */
    rs_canfd_reg32_t data[16]; /* RCFDCnCFDRMDFb_q / CFDRFDFd_x / CFDCFDFd_k */
    unsigned char reserved[52];
} rs_canfd_message_ram_regs_t;

typedef struct st_rs_canfd_tx_buffer_regs
{
    rs_canfd_reg32_t id;       /* RCFDCnCFDTMIDp */
    rs_canfd_reg32_t ptr;      /* RCFDCnCFDTMPTRp */
    rs_canfd_reg32_t fdctr;    /* RCFDCnCFDTMFDCTRp */
    rs_canfd_reg32_t data[16]; /* RCFDCnCFDTMDFb_p */
    unsigned char reserved[52];
} rs_canfd_tx_buffer_regs_t;

typedef struct st_rs_canfd_tx_history_access_regs
{
    rs_canfd_reg32_t acc0; /* RCFDCnCFDTHLACC0m */
    rs_canfd_reg32_t acc1; /* RCFDCnCFDTHLACC1m */
} rs_canfd_tx_history_access_regs_t;

/* The CPU-visible access-window stride is128 bytes, not the internal message
 * RAM budget (12+payload). Check the actual generated header, not tag numbers.
 */
typedef char rs_canfd_tx_stride_check[(sizeof(rs_canfd_tx_buffer_regs_t) ==
                                       offsetof(rs_canfd_unit_t, CFDTMID1) -
                                           offsetof(rs_canfd_unit_t, CFDTMID0))
                                          ? 1
                                          : -1];
typedef char rs_canfd_rx_stride_check[(sizeof(rs_canfd_message_ram_regs_t) ==
                                       offsetof(rs_canfd_unit_t, CFDRFID1) -
                                           offsetof(rs_canfd_unit_t, CFDRFID0))
                                          ? 1
                                          : -1];

#define RS_CANFD_CHANNEL_REGS(unit) ((volatile rs_canfd_channel_regs_t *)&((unit)->CFDC0NCFG))
#define RS_CANFD_CHANNEL_FD_REGS(unit) ((volatile rs_canfd_channel_fd_regs_t *)&((unit)->CFDC0DCFG))
#define RS_CANFD_RX_RULE_REGS(unit) ((volatile rs_canfd_rx_rule_regs_t *)&((unit)->CFDGAFLID0))
#define RS_CANFD_RX_BUFFER_REGS(unit) ((volatile rs_canfd_message_ram_regs_t *)&((unit)->CFDRMID0))
#define RS_CANFD_RX_FIFO_REGS(unit) ((volatile rs_canfd_message_ram_regs_t *)&((unit)->CFDRFID0))
#define RS_CANFD_TXRX_FIFO_REGS(unit) ((volatile rs_canfd_message_ram_regs_t *)&((unit)->CFDCFID0))
#define RS_CANFD_TX_BUFFER_REGS(unit) ((volatile rs_canfd_tx_buffer_regs_t *)&((unit)->CFDTMID0))
#define RS_CANFD_TX_HISTORY_ACC_REGS(unit)                                                         \
    ((volatile rs_canfd_tx_history_access_regs_t *)&((unit)->CFDTHLACC00))

#define RS_CANFD_RX_FIFO_CC(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDRFCC0))
#define RS_CANFD_RX_FIFO_STS(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDRFSTS0))
#define RS_CANFD_RX_FIFO_PCTR(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDRFPCTR0))
#define RS_CANFD_TXRX_FIFO_CC(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDCFCC0))
#define RS_CANFD_TXRX_FIFO_STS(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDCFSTS0))
#define RS_CANFD_TXRX_FIFO_PCTR(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDCFPCTR0))
#define RS_CANFD_TX_BUFFER_TMC(unit) ((volatile unsigned char *)&((unit)->CFDTMC0))
#define RS_CANFD_TX_BUFFER_TMSTS(unit) ((volatile unsigned char *)&((unit)->CFDTMSTS0))
#define RS_CANFD_TX_BUFFER_IE(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDTMIEC0))
#define RS_CANFD_TX_QUEUE_CC(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDTXQCC0))
#define RS_CANFD_TX_QUEUE_STS(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDTXQSTS0))
#define RS_CANFD_TX_QUEUE_PCTR(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDTXQPCTR0))
#define RS_CANFD_TX_HISTORY_CC(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDTHLCC0))
#define RS_CANFD_TX_HISTORY_STS(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDTHLSTS0))
#define RS_CANFD_TX_HISTORY_PCTR(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDTHLPCTR0))
#define RS_CANFD_RX_BUFFER_NEWDATA(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDRMND0))
#define RS_CANFD_RAM_TEST_ACC_REGS(unit) ((volatile rs_canfd_reg32_t *)&((unit)->CFDRPGACC0))

#endif
