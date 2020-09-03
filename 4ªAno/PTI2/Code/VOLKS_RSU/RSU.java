import java.io.IOException;
import java.net.*;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentSkipListMap;
import java.util.concurrent.ConcurrentSkipListSet;

class UDP {
    private int port;
    private InetAddress multicast = InetAddress.getByName("FF02::1");
    private MulticastSocket socket;

    private InetAddress getInterfaceIP() throws SocketException {
        NetworkInterface n = NetworkInterface.getByName("eth1");
        Enumeration<InetAddress> ips = n.getInetAddresses();
        InetAddress ip = null;
        while (ips.hasMoreElements()){
            ip = ips.nextElement();
        }
        return ip;
    }

    public UDP(int port) throws IOException {
        this.port = port;
    }

    public void setUDPSocket() throws IOException{
        socket = new MulticastSocket(port);
        socket.setInterface(getInterfaceIP());
        socket.setLoopbackMode(true);
        socket.setTimeToLive(1);
        socket.setSoTimeout(1500);
    }

    public synchronized void send(byte[] data) throws IOException{
        DatagramPacket packet = new DatagramPacket(data, data.length, this.multicast, this.port);
        this.socket.send(packet);
    }

    public synchronized byte[] read() throws IOException, SocketTimeoutException{
        byte[] data = new byte[256];
        DatagramPacket packet = new DatagramPacket(data,256);
        this.socket.receive(packet);
        return data;
    }

    public boolean isOn() { return !this.socket.isClosed(); }
}

class Packet {
    private ConcurrentHashMap<Integer, LinkedList<byte[]>> packet = new ConcurrentHashMap<>();
    private ConcurrentSkipListSet<Integer> priorityAlert = new ConcurrentSkipListSet<>();
    private ConcurrentSkipListMap<Long, LinkedList<Integer>> alarms = new ConcurrentSkipListMap<>();
    private ConcurrentHashMap<Integer, byte[]> alerts = new ConcurrentHashMap<>();

    private ConcurrentHashMap<Integer, Integer> service = new ConcurrentHashMap<>();

    private UDP udp;
    private Crypto c;
    private int lastPacket = 0;
    private Timer timer = new Timer();


    public Packet(Crypto c){
        this.packet.put(0, new LinkedList<>());
        this.packet.put(1, new LinkedList<>());
        this.c = c;
    }

    public void setUDP(UDP udp){
        this.udp = udp;
    }

    private void setAlarm(int alertID, long end){
        if (alarms.containsKey(end)){
            alarms.get(end).push(alertID);
        }else{
            if (end > System.currentTimeMillis()/1000) {
                alarms.put(end, new LinkedList<>());
                alarms.get(end).push(alertID);
                TimerTask task = new TimerTask() {
                    @Override
                    public void run() {
                        Map.Entry<Long, LinkedList<Integer>> E = alarms.pollFirstEntry();
                        for (Integer i: E.getValue()){
                            if (priorityAlert.contains(i))      priorityAlert.remove(i);
                            alerts.remove(i);
                        }
                        return;
                    }
                };
                timer.schedule(task, end*1000 - System.currentTimeMillis());
            }
        }
    }

    private void addAlert(int alertID, boolean prio, long end, byte[] data){
        alerts.put(alertID, data);
        if (prio)    priorityAlert.add(alertID);
        if (alertID == (lastPacket+1)){
            lastPacket = alertID;
            while(alerts.containsKey(lastPacket + 1))    lastPacket++;
        }
        setAlarm(alertID, end);
    }

    private boolean between(int i, int minValueInclusive, int maxValueInclusive) {
        return (i >= minValueInclusive && i <= maxValueInclusive);
    }

    private void limitService(int[] coord, int dest) throws IOException {
        String info = new String();
        int l = 0;
        if (!this.service.containsKey(dest))    this.service.put(dest, l);
        if (between(coord[1], 0, 40) && between(coord[0], 675, 1645)){
            if (this.service.get(dest) == 70)   return;
            l = 70;
            info = "Moderar velocidade, o limite agora e 70km/h.";
        }else if (between(coord[1], 795, 850) && between(coord[0], 675, 1645)){
            if (this.service.get(dest) == 120)   return;
            l = 120;
            info = "Pode acelerar, o limite agora e 120km/h.";
        }else if (between(coord[1], 41, 794)){
            if (between(coord[0], 625, 675)){
                if (this.service.get(dest) == 100)   return;
                l = 100;
                info = "Pode acelerar, o limite agora e 100km/h.";
            }else if (between(coord[0], 1645, 1670)){
                if (this.service.get(dest) == 90)   return;
                l = 90;
                info = "Moderar velocidade, o limite agora e 90km/h.";
            }else{
                if (this.service.get(dest) == 50)   return;
                l = 50;
                info = "Cuidado, o limite agora e 50km/h.";
            }
        }else{
            return;
        }
        byte[] service = new packetWriter().service((byte) 2, dest, System.currentTimeMillis()/1000, info.getBytes());
        this.service.put(dest, l);
        packet.get(0).push(c.encrypt(service, "volks"));
        udp.send(c.encrypt(service, "car"));
    }

    public int newPacket(byte[] data){
        packetReader p = new packetReader(c.decrypt(data));
        int i = 0;
        switch (p.getType()){
            case 3:
                packet.get(0).push(c.encrypt(p.getPacket(), "volks"));
                try {
                    udp.send(c.encrypt(new packetWriter().ack(p.getHash()),"car"));//ack
                } catch (IOException e) {}
                break;
            case 4:
                i = p.getAlertID();
                if (i > lastPacket)
                    addAlert(i, p.getPriority(), p.getTimestamp() + p.getTTL(), data);
                break;
            case 5:
                i = p.getAlertID();
                try{
                    if (i < lastPacket){
                        int l = p.getLastPriority();
                        try{
                            if (!priorityAlert.isEmpty() && l != priorityAlert.last()){
                                Iterator <Integer> it = priorityAlert.iterator();
                                while(it.hasNext()){
                                    i = it.next();
                                    if (i > l)    break;
                                }
                            }else{
                                i++;
                            }
                            udp.send(alerts.get(i));
                        }catch (NullPointerException e) {
                            udp.send(c.encrypt(new packetWriter().update(i), "car"));
                        }
                    }else{
                        i = p.getCarID();
                        if (packet.containsKey(i)) {
                            if (packet.get(i).size() != 0)
                                udp.send(packet.get(i).pop());
                            else
                                limitService(p.getCoordService(), i);
                        }
                    }
                } catch (IOException e) {}
                break;
            case 7:
                i = p.getDest();
                if (!packet.containsKey(i))   packet.put(i, new LinkedList<>());
                else                          packet.get(i).clear();
                packet.get(i).push(data);
                break;
        }
        return i;
    }
    public boolean hasNewPacket(int id){
        if(packet.get(id).size() == 0)
            return false;
        return true;
    }

    public byte[] getPacket(int id){
        return packet.get(id).pop();
    }
}

class Host_Comm implements Runnable{
    private TCP tcp;
    private Packet packet;
    private Crypto c;
    private int ID;

    public Host_Comm(TCP tcp, Packet packet, int id, Crypto c){
        this.tcp = tcp;
        this.packet = packet;
        this.c = c;
        this.ID = id;
    }

    private void sendID() throws IOException{
        tcp.send(c.encrypt(new packetWriter().rsuID(ID), "volks"));
        ID = 0;
    }

    public void run() {
        byte[] data;
        try {
            sendID();
            while (tcp.isOn()){
                try{
                    data = tcp.read();
                    if (tcp.isOn())    packet.newPacket(data);
                }catch (SocketTimeoutException e){
                    if (packet.hasNewPacket(ID)){
                        data = packet.getPacket(ID);
                        tcp.send(data);
                    }
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        System.exit(0);
    }
}

class Car_Comm implements Runnable{
    private UDP udp;
    private Packet packet;
    private int ID;

    public Car_Comm(UDP udp, Packet packet){
        this.udp = udp;
        this.packet = packet;
        this.ID = 1;
    }
    public void run() {
        byte[] data;
        try {
            udp.setUDPSocket();
            packet.setUDP(udp);
            while (udp.isOn()){
                try{
                    data = udp.read();
                    packet.newPacket(data);
                }catch (SocketTimeoutException e){
                    if (packet.hasNewPacket(ID)){
                        data = packet.getPacket(ID);
                        udp.send(data);
                    }
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

public class RSU{

    public static void main(String[] args) throws IOException{
        Crypto c = new Crypto("car", "1234");
        Packet packet = new Packet(c);

        int id = Integer.parseInt(args[0]);

        Thread host = new Thread(new Host_Comm(new TCP(new Socket("2001:690:2280:823:1000::10",8888)), packet, id, c));
        host.start();

        Thread car = new Thread(new Car_Comm(new UDP(7777), packet));
        car.start();

        try {
            host.join();
            car.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
