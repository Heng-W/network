package pers.hw.net;

public class Buffer {

    private byte[] buf;
    private int prependSize;
    private int readerIndex;
    private int writerIndex;

    public Buffer() {
        this(1024, 8);
    }

    public Buffer(int initialSize) {
        this(initialSize, 8);
    }

    public Buffer(int initialSize, int prependSize) {
        this.prependSize = prependSize;
        this.readerIndex = Math.min(prependSize, initialSize);
        this.writerIndex = readerIndex;
        if (initialSize > 0) {
            buf = new byte[initialSize];
        } else {
            buf = null;
        }
    }

    public int readableBytes() {
        return writerIndex - readerIndex;
    }

    public int writableBytes() {
        return buf.length - writerIndex;
    }

    public int prependableBytes() {
        return readerIndex;
    }

    public int peek() {
        return readerIndex;
    }

    public int beginWrite() {
        return writerIndex;
    }

    public void retrieveAll() {
        readerIndex = writerIndex = prependSize;
    }

    public void hasWritten(int len) {
        writerIndex += len;
    }

    public void unwrite(int len) {
        assert len <= readableBytes();
        writerIndex -= len;
    }

    public byte[] data() {
        return buf;
    }

    public void append(byte[] data) {
        append(data, 0, data.length);
    }

    public void append(byte[] data, int off, int len) {
        ensureWritableBytes(len);
        System.arraycopy(data, off, this.buf, writerIndex, len);
        hasWritten(len);
    }

    public void writeInt8(int num) {
        ensureWritableBytes(1);
        buf[writerIndex++] = (byte) (num & 0xff);
    }

    public void writeInt16(int num) {
        ensureWritableBytes(2);
        buf[writerIndex++] = (byte) ((num >>> 8) & 0xff);
        buf[writerIndex++] = (byte) (num & 0xff);
    }

    public void writeInt32(int num) {
        ensureWritableBytes(4);
        buf[writerIndex++] = (byte) ((num >>> 24) & 0xff);
        buf[writerIndex++] = (byte) ((num >>> 16) & 0xff);
        buf[writerIndex++] = (byte) ((num >>> 8) & 0xff);
        buf[writerIndex++] = (byte) (num & 0xff);
    }

    public void writeInt64(long num) {

        writeInt32((int) (num >>> 32));
        writeInt32((int) (num));
    }

    public int readInt8() {
        return buf[readerIndex++] & 0xff;
    }

    public int readInt16() {
        int res = 0;
        res |= buf[readerIndex++] << 8;
        res |= buf[readerIndex++];
        return res;
    }

    public int readInt32() {
        int res = 0;
        res |= buf[readerIndex++] << 24;
        res |= buf[readerIndex++] << 16;
        res |= buf[readerIndex++] << 8;
        res |= buf[readerIndex++];
        return res;
    }

    public long readInt64() {
        long res = (long) readInt32() << 32;
        res |= readInt32();
        return res;
    }

    public void ensureWritableBytes(int len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
        assert writableBytes() >= len;
    }


    private void makeSpace(int len) {
        int readable = readableBytes();
        if (writableBytes() + prependableBytes() < len + prependSize) {
            int oidSize = prependSize + readable;
            byte[] newBuf = new byte[oidSize + Math.max(oidSize, len)]; // 扩容
            System.arraycopy(buf, readerIndex, newBuf, prependSize, readable);
            buf = newBuf;
        } else {
            System.arraycopy(buf, readerIndex, buf, prependSize, readable); // 数据前移
        }
        readerIndex = prependSize;
        writerIndex = readerIndex + readable;
    }

}
