import java.util.Arrays;

public class packetReader {
    byte[] packet;

    public packetReader(byte[] packet){
            this.packet = packet;
    }

    public byte[] getPacket(){
        return packet;
    }
    public int getType(){
        int type = ((int)(packet[0] & 0xff));
        return type;
    }
    //rsu id packet
    public int getRsuID(){
        int id = ((int)(packet[2] & 0xff)) + ((int)(packet[1] & 0xff)<<8);
        return id;
    }
    //car id packet
    public byte[] getIP(){
        byte[] ip = Arrays.copyOfRange(packet, 1, 17);
        return ip;
    }
    public String getRegistration(){
        byte[] registration = Arrays.copyOfRange(packet,17,23);
        return new String(registration);
    }
    public String getDate(){
        String date = (((int)(packet[25] & 0xff)<<8) + ((int)(packet[26] & 0xff))) + "." +
                ((int)(packet[24] & 0xff)) + "." + ((int)(packet[23] & 0xff));
        return date;
    }
    public int getHash(){
        int hash = ((int)(packet[28] & 0xff))      + ((int)(packet[29] & 0xff)<<8) +
                ((int)(packet[30] & 0xff)<<16)  + ((int)(packet[31] & 0xff)<<24);
        return hash;
    }
    public String[] getModelOwner(){
        byte[] model = Arrays.copyOfRange(packet, 32, 190);
        String m = new String(model);
        return m.split("#");
    }

    //alert packet
    public int getAlertID(){
        int id = ((int)(packet[2] & 0xff))      + ((int)(packet[3] & 0x7f)<<8);
        return id;
    }
    public boolean getPriority(){
        if ((packet[3] & 0x80) == 0x80)    return true;
        return false;
    }
    public long getTimestamp(){
        long timestamp = ((long)(packet[4] & 0xff))      + ((long)(packet[5] & 0xff)<<8) +
                         ((long)(packet[6] & 0xff)<<16)  + ((long)(packet[7] & 0xff)<<24) +
                         ((long)(packet[8] & 0xff)<<32)  + ((long)(packet[9] & 0xff)<<40) +
                         ((long)(packet[10] & 0xff)<<48) + ((long)(packet[11] & 0xff)<<56);
        return timestamp;
    }
    public int getTTL(){
        int ttl = ((int)(packet[12] & 0xff)) + ((int)(packet[13] & 0xff)<<8);
        return ttl;
    }
    public int getCoordXAlert(){
        int coordx = ((int)(packet[14] & 0xff)) + ((int)(packet[15] & 0xff)<<8);
        return coordx;
    }
    public int getCoordYAlert(){
        int coordy = ((int)(packet[16] & 0xff)) + ((int)(packet[17] & 0xff)<<8);
        return coordy;
    }
    public String getInfo() {
        byte[] info = Arrays.copyOfRange(packet, 18, packet.length);
        return new String(info);
    }

    //service packet
    public byte getServiceID(){
        return packet[1];
    }

    public int getDest(){
        int hash = ((int)(packet[4] & 0xff))      + ((int)(packet[5] & 0xff)<<8) +
                ((int)(packet[6] & 0xff)<<16)  + ((int)(packet[7] & 0xff)<<24);
        return hash;
    }
    public long getServiceTimestamp(){
        long timestamp = ((long)(packet[8]  & 0xff))      + ((long)(packet[9]  & 0xff)<<8)  +
                ((long)(packet[10] & 0xff)<<16)  + ((long)(packet[11] & 0xff)<<24) +
                ((long)(packet[12] & 0xff)<<32)  + ((long)(packet[13] & 0xff)<<40) +
                ((long)(packet[14] & 0xff)<<48)  + ((long)(packet[15] & 0xff)<<56);
        return timestamp;
    }
    public String getServiceInfo() {
        byte[] info = Arrays.copyOfRange(packet, 16, packet.length);
        return new String(info);
    }
}
