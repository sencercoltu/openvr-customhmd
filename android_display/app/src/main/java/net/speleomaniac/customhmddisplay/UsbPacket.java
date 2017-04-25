package net.speleomaniac.customhmddisplay;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class UsbPacket {
    final static byte HMD_SOURCE = 0x00;
    final static byte ROTATION_DATA = 0x10;

    byte[] Buffer;
    private ByteBuffer bb;

    public UsbPacket()
    {
        Buffer = new byte[32];
        bb = ByteBuffer.wrap(Buffer);
        bb.order(ByteOrder.LITTLE_ENDIAN);
        Header = new USBDataHeader();
        Rotation = new USBRotationData();
    }

    class USBDataHeader {
        byte Type; //source & data
        short Sequence; //source & data
        byte Crc8; //source & data
    };

    class USBRotationData {
        float w;
        float x;
        float y;
        float z;
    };

    USBDataHeader Header;
    USBRotationData Rotation;


    void buildRotationPacket() {
        synchronized (this) {
            bb.position(0);
            bb.put(Header.Type);
            bb.putShort(Header.Sequence++);
            bb.put(Header.Crc8);

            bb.putFloat(Rotation.w);
            bb.putFloat(Rotation.x);
            bb.putFloat(Rotation.y);
            bb.putFloat(Rotation.z);
        }
    }

}