#include "spi.h"


// BYTE type definition
#ifndef SE8R01_API_H
#define SE8R01_API_H

extern SPI_HandleTypeDef hspi1;
extern SPICommand rfSPICommand; 

//#define SE8R01_READ_REG        0x00  // Define read command to register
//#define SE8R01_WRITE_REG       0x20  // Define write command to register
//#define SE8R01_RD_RX_PLOAD     0x61  // Define RX payload register address
//#define SE8R01_WR_TX_PLOAD     0xA0  // Define TX payload register address
//#define SE8R01_FLUSH_TX        0xE1  // Define flush TX register command
//#define SE8R01_FLUSH_RX        0xE2  // Define flush RX register command
//#define SE8R01_REUSE_TX_PL     0xE3  // Define reuse TX payload register command
//#define SE8R01_NOP             0xFF  // Define No Operation, might be used to read status register

////***************************************************//
//// SPI(nRF24L01) registers(addresses)
//#define SE8R01_CONFIG          0x00  // 'Config' register address
//#define SE8R01_EN_AA           0x01  // 'Enable Auto Acknowledgment' register address
//#define SE8R01_EN_RXADDR       0x02  // 'Enabled RX addresses' register address
//#define SE8R01_SETUP_AW        0x03  // 'Setup address width' register address
//#define SE8R01_SETUP_RETR      0x04  // 'Setup Auto. Retrans' register address
//#define SE8R01_RF_CH           0x05  // 'RF channel' register address
//#define SE8R01_RF_SETUP        0x06  // 'RF setup' register address
//#define SE8R01_STATUS          0x07  // 'Status' register address
//#define SE8R01_OBSERVE_TX      0x08  // 'Observe TX' register address
//#define SE8R01_CD              0x09  // 'Carrier Detect' register address
//#define SE8R01_RX_ADDR_P0      0x0A  // 'RX address pipe0' register address
//#define SE8R01_RX_ADDR_P1      0x0B  // 'RX address pipe1' register address
//#define SE8R01_RX_ADDR_P2      0x0C  // 'RX address pipe2' register address
//#define SE8R01_RX_ADDR_P3      0x0D  // 'RX address pipe3' register address
//#define SE8R01_RX_ADDR_P4      0x0E  // 'RX address pipe4' register address
//#define SE8R01_RX_ADDR_P5      0x0F  // 'RX address pipe5' register address
//#define SE8R01_TX_ADDR         0x10  // 'TX address' register address
//#define SE8R01_RX_PW_P0        0x11  // 'RX payload width, pipe0' register address
//#define SE8R01_RX_PW_P1        0x12  // 'RX payload width, pipe1' register address
//#define SE8R01_RX_PW_P2        0x13  // 'RX payload width, pipe2' register address
//#define SE8R01_RX_PW_P3        0x14  // 'RX payload width, pipe3' register address
//#define SE8R01_RX_PW_P4        0x15  // 'RX payload width, pipe4' register address
//#define SE8R01_RX_PW_P5        0x16  // 'RX payload width, pipe5' register address
//#define SE8R01_FIFO_STATUS     0x17  // 'FIFO Status Register' register address
//#define SE8R01_STA_MARK_RX     0X40
//#define SE8R01_STA_MARK_TX     0X20
//#define SE8R01_STA_MARK_MX     0X10



///////////////////////////////////////////////////////////////////////////////////////
#define iBANK0                  0x00
#define iBANK1                  0x80
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// SE8R01 SPI Commands
//
#define iRF_CMD_READ_REG        0x1F            // 000x xxxx Define read command to register
#define iRF_CMD_WRITE_REG       0x20            // 001x xxxx Define write command to register
#define iRF_CMD_RD_RX_PLOAD     0x61            // 0110 0001 Define RX payload register address
#define iRF_CMD_WR_TX_PLOAD     0xA0            // 1010 0000 Define TX payload register address
#define iRF_CMD_FLUSH_TX        0xE1            // 1110 0001 Define flush TX register command
#define iRF_CMD_FLUSH_RX        0xE2            // 1110 0010 Define flush RX register command
#define iRF_CMD_REUSE_TX_PL     0xE3            // 1110 0011 Define reuse TX payload register command
#define iRF_CMD_W_TX_PAYLOAD_NOACK 0xB0         // 1011 0000
#define iRF_CMD_W_ACK_PAYLOAD   0xa8            // 1010 1xxx
#define iRF_CMD_ACTIVATE        0x50            // 0101 0000
#define iRF_CMD_R_RX_PL_WID     0x60            // 0110 0000
#define iRF_CMD_NOP             0xFF            // 1111 1111 Define No Operation, might be used to read status register
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// SE8R01 registers addresses
//
#define iRF_BANK0_CONFIG        0x00            // 'Config' register address
#define iRF_BANK0_EN_AA         0x01            // 'Enable Auto Acknowledgment' register address
#define iRF_BANK0_EN_RXADDR     0x02            // 'Enabled RX addresses' register address
#define iRF_BANK0_SETUP_AW      0x03            // 'Setup address width' register address
#define iRF_BANK0_SETUP_RETR    0x04            // 'Setup Auto. Retrans' register address
#define iRF_BANK0_RF_CH         0x05            // 'RF channel' register address
#define iRF_BANK0_RF_SETUP      0x06            // 'RF setup' register address
#define iRF_BANK0_STATUS        0x07            // 'Status' register address
#define iRF_BANK0_OBSERVE_TX    0x08            // 'Observe TX' register address
#define iRF_BANK0_RPD           0x09            // 'Received Power Detector' register address
#define iRF_BANK0_RX_ADDR_P0    0x0A            // 'RX address pipe0' register address
#define iRF_BANK0_RX_ADDR_P1    0x0B            // 'RX address pipe1' register address
#define iRF_BANK0_RX_ADDR_P2    0x0C            // 'RX address pipe2' register address
#define iRF_BANK0_RX_ADDR_P3    0x0D            // 'RX address pipe3' register address
#define iRF_BANK0_RX_ADDR_P4    0x0E            // 'RX address pipe4' register address
#define iRF_BANK0_RX_ADDR_P5    0x0F            // 'RX address pipe5' register address
#define iRF_BANK0_TX_ADDR       0x10            // 'TX address' register address
#define iRF_BANK0_RX_PW_P0      0x11            // 'RX payload width, pipe0' register address
#define iRF_BANK0_RX_PW_P1      0x12            // 'RX payload width, pipe1' register address
#define iRF_BANK0_RX_PW_P2      0x13            // 'RX payload width, pipe2' register address
#define iRF_BANK0_RX_PW_P3      0x14            // 'RX payload width, pipe3' register address
#define iRF_BANK0_RX_PW_P4      0x15            // 'RX payload width, pipe4' register address
#define iRF_BANK0_RX_PW_P5      0x16            // 'RX payload width, pipe5' register address
#define iRF_BANK0_FIFO_STATUS   0x17            // 'FIFO Status Register' register address
#define iRF_BANK0_DYNPD         0x1C            // 'Enable dynamic payload length' register address
#define iRF_BANK0_FEATURE       0x1D            // 'Feature' register address
#define iRF_BANK0_SETUP_VALUE   0x1E
#define iRF_BANK0_PRE_GURD      0x1F

//SE8R01 Bank1 register
#define iRF_BANK1_LINE          0x00
#define iRF_BANK1_PLL_CTL0      0x01
#define iRF_BANK1_PLL_CTL1      0x02
#define iRF_BANK1_CAL_CTL       0x03
#define iRF_BANK1_A_CNT_REG     0x04
#define iRF_BANK1_B_CNT_REG     0x05
#define iRF_BANK1_RESERVED0     0x06
#define iRF_BANK1_STATUS        0x07
#define iRF_BANK1_STATE         0x08
#define iRF_BANK1_CHAN          0x09
#define iRF_BANK1_IF_FREQ       0x0A
#define iRF_BANK1_AFC_COR       0x0B
#define iRF_BANK1_FDEV          0x0C
#define iRF_BANK1_DAC_RANGE     0x0D
#define iRF_BANK1_DAC_IN        0x0E
#define iRF_BANK1_CTUNING       0x0F
#define iRF_BANK1_FTUNING       0x10
#define iRF_BANK1_RX_CTRL       0x11
#define iRF_BANK1_FAGC_CTRL     0x12
#define iRF_BANK1_FAGC_CTRL_1   0x13
#define iRF_BANK1_DAC_CAL_LOW   0x17
#define iRF_BANK1_DAC_CAL_HI    0x18
#define iRF_BANK1_RESERVED1     0x19
#define iRF_BANK1_DOC_DACI      0x1A
#define iRF_BANK1_DOC_DACQ      0x1B
#define iRF_BANK1_AGC_CTRL      0x1C
#define iRF_BANK1_AGC_GAIN      0x1D
#define iRF_BANK1_RF_IVGEN      0x1E
#define iRF_BANK1_TEST_PKDET    0x1F
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// SE8R01 interrupt status
//
#define iSTATUS_RX_DR           0x40
#define iSTATUS_TX_DS           0x20
#define iSTATUS_MAX_RT          0x10
#define iSTATUS_TX_FULL         0x01
#define iSTATUS_RX_EMPTY        0x0e
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// SE8R01 FIFO status
//
#define iFIFO_STATUS_TX_REUSE   0x40
#define iFIFO_STATUS_TX_FULL    0x20
#define iFIFO_STATUS_TX_EMPTY   0x10
        
#define iFIFO_STATUS_RX_FULL    0x02
#define iFIFO_STATUS_RX_EMPTY   0x01
////////////////////////////////////////////////	    

enum SE8R01_RFMode
{
	SE8R01_RFMode_Receive = 0,
	SE8R01_RFMode_Transmit
};

extern SE8R01_RFMode SE8R01_OPERATE_MODE;

extern bool SE8R01_Init();
extern bool SE8R01_SwitchBank(uint8_t bankindex);
extern bool SE8R01_ClearInterrupts();
extern bool SE8R01_FlushBuffers();
extern bool SE8R01_ReadPayload(uint8_t* pBuf);
extern bool SE8R01_WritePayload(uint8_t* pBuf, bool ack);
extern uint8_t SE8R01_ReadStatus();
	
#endif //SE8R01_

