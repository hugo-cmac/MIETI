public class packetWriter {
    public byte[] alert(int alertID, boolean prio, long timestamp, int ttl, int coordX, int coordY, byte[] info){
        byte[] packet = new byte[190];
        //packet ID
        packet[0]  = 4;
        //alert ID
        packet[2]  = (byte) ((alertID) & 0xff);             packet[3]  = (byte) ((alertID>>8) & 0xff);
        //prio
        if (prio)    packet[3] = (byte) (packet[3] | 0x80);
        //timestamp
        packet[4]  = (byte) ((timestamp) & 0xff);           packet[5]  = (byte) ((timestamp >> 8) & 0xff);
        packet[6]  = (byte) ((timestamp >> 16) & 0xff);     packet[7]  = (byte) ((timestamp >> 24) & 0xff);
        packet[8]  = (byte) ((timestamp >> 32) & 0xff);     packet[9]  = (byte) ((timestamp >> 40) & 0xff);
        packet[10] = (byte) ((timestamp >> 48) & 0xff);     packet[11] = (byte) ((timestamp >> 56) & 0xff);
        //ttl
        packet[12] = (byte) ((ttl) & 0xff);                 packet[13] = (byte) ((ttl>>8) & 0xff);
        //coordX
        packet[14] = (byte) ((coordX) & 0xff);              packet[15] = (byte) ((coordX>>8) & 0xff);
        //coordY
        packet[16] = (byte) ((coordY) & 0xff);              packet[17] = (byte) ((coordY>>8) & 0xff);
        //info
        System.arraycopy(info, 0, packet, 18, info.length);
        return packet;
    }
}
