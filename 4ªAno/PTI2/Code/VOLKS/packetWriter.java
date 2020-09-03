public class packetWriter {
    public byte[] brandID(byte[] brandID, byte[] ip){
        byte[] packet = new byte[190];
        //packet ID
        packet[0] = 1;
        //brand ip
        System.arraycopy(ip, 0, packet, 1, ip.length);
        //brand ID
        System.arraycopy(brandID, 0, packet, 18, brandID.length);
        return packet;
    }

    public byte[] service(byte serviceID, int dest, long timestamp, byte[] info){
        byte[] packet = new byte[190];
        //packet ID
        packet[0] = 7;
        //service ID
        packet[1] = serviceID;
        //dest
        packet[4]  = (byte) ((dest) & 0xff);           packet[5]  = (byte) ((dest >> 8) & 0xff);
        packet[6]  = (byte) ((dest >> 16) & 0xff);     packet[7]  = (byte) ((dest >> 24) & 0xff);
        //timestamp
        packet[8]   = (byte) ((timestamp) & 0xff);           packet[9]   = (byte) ((timestamp >> 8) & 0xff);
        packet[10]  = (byte) ((timestamp >> 16) & 0xff);     packet[11]  = (byte) ((timestamp >> 24) & 0xff);
        packet[12]  = (byte) ((timestamp >> 32) & 0xff);     packet[13]  = (byte) ((timestamp >> 40) & 0xff);
        packet[14]  = (byte) ((timestamp >> 48) & 0xff);     packet[15]  = (byte) ((timestamp >> 56) & 0xff);
        //info
        System.arraycopy(info, 0, packet, 16, info.length);
        return packet;
    }
}
