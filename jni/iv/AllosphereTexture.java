package iv;

import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class AllosphereTexture {

    public String name;
    public int width;
    public int height;

    // The program will receive parameters from the main rendering process.
    // -ivTexture:key=shm_id,sem_id,size,width,height
    public static AllosphereTexture ParseArgument(String param) {
        Pattern pattern = Pattern.compile("\\-ivTexture\\:([0-9a-zA-Z\\_\\-]+)\\=(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)");
        Matcher m = pattern.matcher(param);
        if(m.find()) {
            String name = m.group(1);
            int shm_id = Integer.parseInt(m.group(2));
            int sem_id = Integer.parseInt(m.group(3));
            int size = Integer.parseInt(m.group(4));
            int width = Integer.parseInt(m.group(5));
            int height = Integer.parseInt(m.group(6));
            AllosphereTexture result = new AllosphereTexture(shm_id, sem_id, size);
            result.name = name;
            result.width = width;
            result.height = height;
            return result;
        } else {
            return null;
        }
    }

    private AllosphereTexture(int shm_id, int sem_id, int size) {
        if(!native_library_loaded) return;
        native_ptr = open_shared_memory(shm_id, sem_id, size);
        if(native_ptr == 0) {
            System.out.println("Warning: shmat() failed, seems the shared memory doesn't exist.");
        }
    }

    public void upload(int texture_id) {
        if(!native_library_loaded || native_ptr == 0) return;
        upload_texture(native_ptr, texture_id, width, height);
    }
    public void upload(java.nio.ByteBuffer buffer) {
        if(!native_library_loaded || native_ptr == 0) return;
        upload_buffer(native_ptr, buffer, width, height);
    }

    public void finalize() {
        if(!native_library_loaded || native_ptr == 0) return;
        close_shared_memory(native_ptr);
        native_ptr = 0;
    }

    private static boolean native_library_loaded = false;
    static {
        try {
            System.loadLibrary("IVAllosphereTexture");
            native_library_loaded = true;
        } catch(java.lang.UnsatisfiedLinkError e) {
            System.out.println("Warning: Unable to load the native library, switch to dummy mode.");
        }
    }

    private static native long open_shared_memory(int shm_id, int sem_id, int size);
    private static native void close_shared_memory(long ptr);
    private static native void upload_texture(long ptr, int texture_id, int width, int height);
    private static native void upload_buffer(long ptr, java.nio.ByteBuffer buffer, int width, int height);

    private long native_ptr;
}
