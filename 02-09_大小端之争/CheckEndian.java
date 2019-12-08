import java.nio.ByteBuffer;

public class CheckEndian
{
    public static void checkEndian()
    {
        int x = 0xAABBCCDD;

        ByteBuffer buffer = ByteBuffer.allocate(Integer.BYTES);
        buffer.putInt(x);
        byte[] lbytes = buffer.array();
        for (byte b : lbytes){
            System.out.printf("%X\n", b);
        }
    }
    public static void main(String[] args)
    {
        checkEndian();
    }
}