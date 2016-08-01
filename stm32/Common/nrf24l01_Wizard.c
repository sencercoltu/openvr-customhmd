#include "nrf24l01.h"
// <<< Use Configuration Wizard in Context Menu >>>
void NRF24L01_Configuration (RF_InitTypeDef* RF_InitStruct)	
{	
// ================================ CONFIGURATION REGISTERS =========================================
//		
//		<h> Basic configuration
//				<o0.0> Mode selection:  
//					<1=> Receiver 
//					<0=> Transmitter	
//				<o0.1> Power control
//					<1=> Ready
//					<0=> Suspend mode
//				<o0.2..3> CRC
//					<0=> CRC off
//					<1=> CRC8 on
//					<3=> CRC16 on
//				<o0.4> Maximum Retrasmit Interrupt Enable 
//				<o0.5> Data Send Interrupt Enable
//				<o0.6> Data Received Interrupt Enable
//				<o1.0..1> Payload Address Lenght
//					<1=> 3 bytes
//					<2=> 4 bytes
//					<3=> 5 bytes
//				<o2.0..6> Frequency Channel 0...127
//				<o3.3>	Data Rate
//					<0=> 1 Mbps
//					<1=> 2 Mbps
//				<o3.1..2> Output Power
//					<0=> -18 dBm
//					<1=> -12 dBm
//					<2=> -6 dBm
//					<3=> 0 dBm
//		</h>	
#define RFW_CONFIG_REG					79
#define RFW_SETUP_AW_REG				3
#define RFW_RF_CH_REG						53
#define RFW_RF_SETUP_REG				6
//		<h> Auto Acknowledgment Selection
//				<o0.0> Enable auto ack. data pipe 0
//				<o0.1> Enable auto ack. data pipe 1
//				<o0.2> Enable auto ack. data pipe 2
//				<o0.3> Enable auto ack. data pipe 3
//				<o0.4> Enable auto ack. data pipe 4
//				<o0.5> Enable auto ack. data pipe 5
//		</h>
#define RFW_EN_AA_REG				3
//		<h> Enable RX Data Pipes
//				<o0.0> Enable Data Pipe 0
//				<o0.1> Enable Data Pipe 1
//				<o0.2> Enable Data Pipe 2
//				<o0.3> Enable Data Pipe 3
//				<o0.4> Enable Data Pipe 4
//				<o0.5> Enable Data Pipe 5
//		</h>
#define RFW_EN_RXADDR_REG			3
//		<h> Automatic Retrasmit Adjustment
//				<o0.4..7> Automatic Retrasmit Delay
//						<0=> Wait 250+86uS
//						<1=> Wait 500+86uS 
//						<2=> Wait 750+86uS 
//						<3=> Wait 1000+86uS 
//						<4=> Wait 1250+86uS 
//						<5=> Wait 1500+86uS 
//						<6=> Wait 1750+86uS 
//						<7=> Wait 2000+86uS 
//						<8=> Wait 2250+86uS 
//						<9=> Wait 2500+86uS 
//						<10=> Wait 2750+86uS 
//						<11=> Wait 3000+86uS 
//						<12=> Wait 3250+86uS 
//						<13=> Wait 3500+86uS 
//						<14=> Wait 3750+86uS 
//						<15=> Wait 4000+86uS
//				<o0.0..3> Auto Retransmit Count
//						<0=> Re-Transmit disabled
//						<1=> 1
//						<2=> 2
//						<3=> 3
//						<4=> 4
//						<5=> 5
//						<6=> 6
//						<7=> 7
//						<8=> 8
//						<9=> 9
//						<10=> 10
//						<11=> 11
//						<12=> 12
//						<13=> 13
//						<14=> 14
//						<15=> 15
//		</h>
#define RFW_SETUP_RETR_REG 			0
//		<h> Addresses definition
//			<s0.5> TX Address
//			<s1.5> Pipe 0 Address
//			<s2.5> Pipe 1 Address
//			<s3.1> Pipe 2 MSB Address
//			<s4.1> Pipe 3 MSB Address
//			<s5.1> Pipe 4 MSB Address
//			<s6.1> Pipe 5 MSB Address
//		</h>
#define RFW_TX_ADDR_REG "KEFIR"
#define RFW_RX_ADDR_P0_REG "BATON"
#define RFW_RX_ADDR_P1_REG "GAMON"
#define RFW_RX_ADDR_P2_REG "1"
#define RFW_RX_ADDR_P3_REG "2"
#define RFW_RX_ADDR_P4_REG "3"
#define RFW_RX_ADDR_P5_REG "4"
//		<h> Payloads Size Definition
//			<o0.0..5> Number of bytes in RX payload in data pipe 0
//						<0=> Data Pipe 0 disabled
//						<1=> 1
//						<2=> 2
//						<3=> 3
//						<4=> 4
//						<5=> 5
//						<6=> 6
//						<7=> 7
//						<8=> 8
//						<9=> 9
//						<10=> 10
//						<11=> 11
//						<12=> 12
//						<13=> 13
//						<14=> 14
//						<15=> 15		
//						<16=> 16		
//						<17=> 17		
//						<18=> 18		
//						<19=> 19		
//						<20=> 20		
//						<21=> 21		
//						<22=> 22		
//						<23=> 23		
//						<24=> 24		
//						<25=> 25		
//						<26=> 26		
//						<27=> 27		
//						<28=> 28		
//						<29=> 29		
//						<30=> 30		
//						<31=> 31		
//						<32=> 32		
//			<o1.0..5> Number of bytes in RX payload in data pipe 1
//						<0=> Data Pipe 1 disabled
//						<1=> 1
//						<2=> 2
//						<3=> 3
//						<4=> 4
//						<5=> 5
//						<6=> 6
//						<7=> 7
//						<8=> 8
//						<9=> 9
//						<10=> 10
//						<11=> 11
//						<12=> 12
//						<13=> 13
//						<14=> 14
//						<15=> 15		
//						<16=> 16		
//						<17=> 17		
//						<18=> 18		
//						<19=> 19		
//						<20=> 20		
//						<21=> 21		
//						<22=> 22		
//						<23=> 23		
//						<24=> 24		
//						<25=> 25		
//						<26=> 26		
//						<27=> 27		
//						<28=> 28		
//						<29=> 29		
//						<30=> 30		
//						<31=> 31		
//						<32=> 32		
//			<o2.0..5> Number of bytes in RX payload in data pipe 2
//						<0=> Data Pipe 2 disabled
//						<1=> 1
//						<2=> 2
//						<3=> 3
//						<4=> 4
//						<5=> 5
//						<6=> 6
//						<7=> 7
//						<8=> 8
//						<9=> 9
//						<10=> 10
//						<11=> 11
//						<12=> 12
//						<13=> 13
//						<14=> 14
//						<15=> 15		
//						<16=> 16		
//						<17=> 17		
//						<18=> 18		
//						<19=> 19		
//						<20=> 20		
//						<21=> 21		
//						<22=> 22		
//						<23=> 23		
//						<24=> 24		
//						<25=> 25		
//						<26=> 26		
//						<27=> 27		
//						<28=> 28		
//						<29=> 29		
//						<30=> 30		
//						<31=> 31		
//						<32=> 32		
//			<o3.0..5> Number of bytes in RX payload in data pipe 3
//						<0=> Data Pipe 3 disabled
//						<1=> 1
//						<2=> 2
//						<3=> 3
//						<4=> 4
//						<5=> 5
//						<6=> 6
//						<7=> 7
//						<8=> 8
//						<9=> 9
//						<10=> 10
//						<11=> 11
//						<12=> 12
//						<13=> 13
//						<14=> 14
//						<15=> 15		
//						<16=> 16		
//						<17=> 17		
//						<18=> 18		
//						<19=> 19		
//						<20=> 20		
//						<21=> 21		
//						<22=> 22		
//						<23=> 23		
//						<24=> 24		
//						<25=> 25		
//						<26=> 26		
//						<27=> 27		
//						<28=> 28		
//						<29=> 29		
//						<30=> 30		
//						<31=> 31		
//						<32=> 32		
//			<o4.0..5> Number of bytes in RX payload in data pipe 4
//						<0=> Data Pipe 4 disabled
//						<1=> 1
//						<2=> 2
//						<3=> 3
//						<4=> 4
//						<5=> 5
//						<6=> 6
//						<7=> 7
//						<8=> 8
//						<9=> 9
//						<10=> 10
//						<11=> 11
//						<12=> 12
//						<13=> 13
//						<14=> 14
//						<15=> 15		
//						<16=> 16		
//						<17=> 17		
//						<18=> 18		
//						<19=> 19		
//						<20=> 20		
//						<21=> 21		
//						<22=> 22		
//						<23=> 23		
//						<24=> 24		
//						<25=> 25		
//						<26=> 26		
//						<27=> 27		
//						<28=> 28		
//						<29=> 29		
//						<30=> 30		
//						<31=> 31		
//						<32=> 32
//			<o5.0..5> Number of bytes in RX payload in data pipe 5
//						<0=> Data Pipe 5 disabled
//						<1=> 1
//						<2=> 2
//						<3=> 3
//						<4=> 4
//						<5=> 5
//						<6=> 6
//						<7=> 7
//						<8=> 8
//						<9=> 9
//						<10=> 10
//						<11=> 11
//						<12=> 12
//						<13=> 13
//						<14=> 14
//						<15=> 15		
//						<16=> 16		
//						<17=> 17		
//						<18=> 18		
//						<19=> 19		
//						<20=> 20		
//						<21=> 21		
//						<22=> 22		
//						<23=> 23		
//						<24=> 24		
//						<25=> 25		
//						<26=> 26		
//						<27=> 27		
//						<28=> 28		
//						<29=> 29		
//						<30=> 30		
//						<31=> 31		
//						<32=> 32		
//		</h>
#define RFW_RX_PW_P0_REG 16
#define RFW_RX_PW_P1_REG 16
#define RFW_RX_PW_P2_REG 0
#define RFW_RX_PW_P3_REG 0
#define RFW_RX_PW_P4_REG 0
#define RFW_RX_PW_P5_REG 0

	RF_InitStruct->RF_Power_State=RFW_CONFIG_REG&1;
	RF_InitStruct->RF_Config=(RFW_CONFIG_REG&0x70)^0x70;
	RF_InitStruct->RF_CRC_Mode=RFW_CONFIG_REG&0x0C;
	RF_InitStruct->RF_Mode=RFW_CONFIG_REG&2;
	RF_InitStruct->RF_Pipe_Auto_Ack=RFW_EN_AA_REG;
	RF_InitStruct->RF_Enable_Pipe=RFW_EN_RXADDR_REG;
	RF_InitStruct->RF_Setup=RFW_SETUP_AW_REG&0x03;
	RF_InitStruct->RF_TX_Power=RFW_RF_SETUP_REG&0x06;
	RF_InitStruct->RF_Data_Rate=RFW_RF_SETUP_REG&(1<<3);
	RF_InitStruct->RF_Channel=RFW_RF_CH_REG;
	RF_InitStruct->RF_RX_Adress_Pipe0[0]=RFW_RX_ADDR_P0_REG[0];
	RF_InitStruct->RF_RX_Adress_Pipe0[1]=RFW_RX_ADDR_P0_REG[1];
	RF_InitStruct->RF_RX_Adress_Pipe0[2]=RFW_RX_ADDR_P0_REG[2];
	RF_InitStruct->RF_RX_Adress_Pipe0[3]=RFW_RX_ADDR_P0_REG[3];
	RF_InitStruct->RF_RX_Adress_Pipe0[4]=RFW_RX_ADDR_P0_REG[4];
	
	RF_InitStruct->RF_RX_Adress_Pipe1[0]=RFW_RX_ADDR_P1_REG[0];
	RF_InitStruct->RF_RX_Adress_Pipe1[1]=RFW_RX_ADDR_P1_REG[1];
	RF_InitStruct->RF_RX_Adress_Pipe1[2]=RFW_RX_ADDR_P1_REG[2];
	RF_InitStruct->RF_RX_Adress_Pipe1[3]=RFW_RX_ADDR_P1_REG[3];
	RF_InitStruct->RF_RX_Adress_Pipe1[4]=RFW_RX_ADDR_P1_REG[4];
	
	RF_InitStruct->RF_RX_Adress_Pipe2=RFW_RX_ADDR_P2_REG[0];
	RF_InitStruct->RF_RX_Adress_Pipe3=RFW_RX_ADDR_P3_REG[0];
	RF_InitStruct->RF_RX_Adress_Pipe4=RFW_RX_ADDR_P4_REG[0];
	RF_InitStruct->RF_RX_Adress_Pipe5=RFW_RX_ADDR_P5_REG[0];
	
	RF_InitStruct->RF_TX_Adress[0]=RFW_TX_ADDR_REG[0];
	RF_InitStruct->RF_TX_Adress[1]=RFW_TX_ADDR_REG[1];
	RF_InitStruct->RF_TX_Adress[2]=RFW_TX_ADDR_REG[2];
	RF_InitStruct->RF_TX_Adress[3]=RFW_TX_ADDR_REG[3];
	RF_InitStruct->RF_TX_Adress[4]=RFW_TX_ADDR_REG[4];
	
	RF_InitStruct->RF_Payload_Size_Pipe0=RFW_RX_PW_P0_REG;
	RF_InitStruct->RF_Payload_Size_Pipe1=RFW_RX_PW_P1_REG;
	RF_InitStruct->RF_Payload_Size_Pipe2=RFW_RX_PW_P2_REG;
	RF_InitStruct->RF_Payload_Size_Pipe3=RFW_RX_PW_P3_REG;
	RF_InitStruct->RF_Payload_Size_Pipe4=RFW_RX_PW_P4_REG;
	RF_InitStruct->RF_Payload_Size_Pipe5=RFW_RX_PW_P5_REG;
	
	RF_InitStruct->RF_Auto_Retransmit_Count=RFW_SETUP_RETR_REG&0x0F;
	RF_InitStruct->RF_Auto_Retransmit_Delay=RFW_SETUP_RETR_REG&0xF0;
}
// <<< end of configuration section >>>

