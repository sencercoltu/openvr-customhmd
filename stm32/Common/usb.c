
#ifdef __cplusplus
extern "C" {
#endif

#include "usb.h"

	uint8_t USB_RX_Buffer[CUSTOM_HID_EPIN_SIZE] = { 0 };

	void SetPacketCrc(struct USBPacket *pPacket)
	{
		pPacket->Header.Crc8 = 0;
		uint8_t* data = (uint8_t*)pPacket;
		uint8_t crc = 0;
		for (int i = 0; i < sizeof(struct USBPacket); i++)
			crc ^= data[i];
		pPacket->Header.Crc8 = crc;
	}

	uint8_t GetPacketCrc(struct USBPacket *pPacket)
	{
		uint8_t crcTemp = pPacket->Header.Crc8;
		pPacket->Header.Crc8 = 0;
		uint8_t* data = (uint8_t*)pPacket;
		uint8_t crc = 0;
		for (int i = 0; i < sizeof(struct USBPacket); i++)
			crc ^= data[i];
		pPacket->Header.Crc8 = crcTemp;
		return crc;
	}

	uint8_t CheckPacketCrc(struct USBPacket *pPacket)
	{
		return pPacket->Header.Crc8 == GetPacketCrc(pPacket);
	}

#ifdef __cplusplus
}
#endif
