import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.SocketTimeoutException;

public class TCP {
    private Socket socket;
    private boolean connection;

    public TCP(Socket socket) throws IOException{
        this.socket = socket;
        this.socket.setSoTimeout(100);
        this.connection = true;
    }

    public byte[] read() throws IOException, SocketTimeoutException {
        byte[] data = new byte[256];
        DataInputStream packet = new DataInputStream(socket.getInputStream());
        if (packet.read(data) < 1)
            connection = false;
        return data;
    }

    public void send(byte[] data) throws IOException{
        DataOutputStream packet = new DataOutputStream(socket.getOutputStream());
        packet.write(data);
    }

    public void close() throws IOException{ socket.close(); }

    public boolean isOn(){
        return connection;
    }
}
