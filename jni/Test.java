import java.nio.ByteBuffer;

public class Test {
    public static void main(String argv[]) throws InterruptedException {
        iv.AllosphereTexture tex = null;
        for(int i = 0; i < argv.length; i++) {
            tex = iv.AllosphereTexture.ParseArgument(argv[i]);
            if(tex != null) break;
        }
        if(tex == null) return;
        System.out.println(tex.name);
        ByteBuffer pixels = ByteBuffer.allocateDirect(tex.width * tex.height * 4);
        // Fill color.
        java.util.Random rnd = new java.util.Random();
        while(true) {
            byte[] rgb = new byte[3];
            rnd.nextBytes(rgb);
            for(int x = 0; x < tex.width; x++) for(int y = 0; y < tex.height; y++) {
                int pos = (y * tex.width + x) * 4;
                pixels.put(pos + 0, rgb[0]);
                pixels.put(pos + 1, rgb[1]);
                pixels.put(pos + 2, rgb[2]);
                pixels.put(pos + 3, (byte)255);
            }
            tex.upload(pixels);
            Thread.sleep(200);
        }

        // To submit an OpenGL texture, use:
        // tex.upload(texture_id);
    }
}
