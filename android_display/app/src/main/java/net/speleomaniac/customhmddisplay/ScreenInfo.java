package net.speleomaniac.customhmddisplay;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;


class ScreenInfo {
    public final int ClassSize = 16;
    int Updated;
    int Stride;
    int Width;
    int Height;

    ScreenInfo(byte[] bytes) {
        ByteBuffer bb = ByteBuffer.wrap(bytes);
        bb.order(ByteOrder.LITTLE_ENDIAN); // or BIG_ENDIAN
        Updated = bb.getInt();
        Stride = bb.getInt();
        Width = bb.getInt();
        Height = bb.getInt();
    }
}
