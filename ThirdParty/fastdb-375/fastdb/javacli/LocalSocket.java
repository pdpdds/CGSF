package javacli;


import java.net.*;
import java.io.*;


public class LocalSocket extends Socket { 
    private long implObj;

    class LocalInputStream extends InputStream { 
        public int read() throws IOException {
            byte buf[] = new byte[1];
            return (read(buf, 0, 1) == 1) ? (buf[0] & 0xFF) : -1;
        }
        public int read(byte b[], int off, int len) throws IOException { 
            if (implObj == 0) { 
                throw new SocketException("Socket is not connected");
            }
            return readSocket(implObj, b, off, len);
        }
    }

    class LocalOutputStream extends OutputStream { 
        public void write(int b) throws IOException {
            byte buf[] = new byte[1];
            buf[0] = (byte)b;
            write(buf, 0, 1);
        }
        
        public void write(byte b[], int off, int len) throws IOException {
            if (implObj == 0) { 
                throw new SocketException("Socket is not connected");
            }
            writeSocket(implObj, b, off, len);
        }
    }

    public InputStream getInputStream() throws IOException {
        return new LocalInputStream();
    }
     
    public OutputStream getOutputStream() throws IOException {
        return new LocalOutputStream();
    }

    public void setTcpNoDelay(boolean on) {}
    public void setSoLinger(boolean on, int linger) {}

    public void open(String address, int port) {
        implObj = openSocket(address, port);
    }

    public void close() { 
        closeSocket(implObj);
        implObj = 0;
    }

    public LocalSocket accept() throws SocketException {
        return new LocalSocket(acceptSocket(implObj));
    }

    public void cancelAccept() { 
        cancelAcceptSocket(implObj);
    }

    public static LocalSocket create(int port) throws SocketException { 
        return new LocalSocket(createSocket(port));
    }

    public LocalSocket(long implObj) throws SocketException { 
        super((SocketImpl)null);
        this.implObj = implObj;
    }

    public LocalSocket(String address, int port) throws SocketException { 
        super((SocketImpl)null);
        open(address, port);
    }

    public LocalSocket() throws SocketException {
        super((SocketImpl)null);
    }

    private static native int  readSocket(long implObj, byte b[], int off, int len);
    private static native void writeSocket(long implObj, byte b[], int off, int len);

    private static native void closeSocket(long implObj);
    private static native long openSocket(String address, int port);

    private static native long createSocket(int port);
    private static native long acceptSocket(long implObj);
    private static native void cancelAcceptSocket(long implObj);

    static { 
        Runtime.getRuntime().loadLibrary("jnilocalsock");
    }
};


