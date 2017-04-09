package net.speleomaniac.customhmddisplay;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class ScreenPartInfo {
    final static int ClassSize = 8;
    int Eye;
    int Size;


    ScreenPartInfo(byte[] bytes) {
        ByteBuffer bb = ByteBuffer.wrap(bytes);
        bb.order(ByteOrder.LITTLE_ENDIAN); // or BIG_ENDIAN
        Eye = bb.getInt();
        Size = bb.getInt();
    }
}
