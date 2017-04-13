package net.speleomaniac.customhmddisplay;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

enum RemotePacketype
{
    RPT_Trash,
    RPT_Start,
    RPT_Payload,
    RPT_End
};


class RemoteFrameType {
    final static int ClassSize = 4;
    int Type;

    RemoteFrameType(byte[] bytes) {
        ByteBuffer bb = ByteBuffer.wrap(bytes);
        bb.order(ByteOrder.LITTLE_ENDIAN); // or BIG_ENDIAN
        Type = bb.getInt();
    }
}


class RemoteFrameStart {
    final static int ClassSize = 4 * 4;
    int Eye;
    int Width;
    int Height;
    int Stride;
    int Size;

    RemoteFrameStart(byte[] bytes) {
        ByteBuffer bb = ByteBuffer.wrap(bytes);
        bb.order(ByteOrder.LITTLE_ENDIAN); // or BIG_ENDIAN
        Eye = bb.getInt();
        Width = bb.getInt();
        Height = bb.getInt();
        Stride = bb.getInt();
        Size = Height * Stride;
    }
}

class RemoteFramePayload {
    final static int ClassSize = 4;
    int PartSize;

    RemoteFramePayload(byte[] bytes) {
        ByteBuffer bb = ByteBuffer.wrap(bytes);
        bb.order(ByteOrder.LITTLE_ENDIAN); // or BIG_ENDIAN
        PartSize = bb.getInt();
    }
}


class RemoteFrameEnd {
    final static int ClassSize = 4;
    int Size;

    RemoteFrameEnd(byte[] bytes) {
        ByteBuffer bb = ByteBuffer.wrap(bytes);
        bb.order(ByteOrder.LITTLE_ENDIAN); // or BIG_ENDIAN
        Size = bb.getInt();
    }
}