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
    private ConcurrentHashMap<int[], Integer> alertSite = new ConcurrentHashMap<>();

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
                            if (alertSite.containsValue(i))     alertSite.forEach((ints, integer) -> { if (integer == i)   alertSite.remove(ints); });
                            alerts.remove(i);
                        }
                        return;
                    }
                };
                timer.schedule(task, end*1000 - System.currentTimeMillis());
            }
        }
    }

    private void addAlert(int alertID, boolean prio, long end, int[] coord, byte[] data){
        alerts.put(alertID, data);
        if (prio)    priorityAlert.add(alertID);
        if (alertID == (lastPacket+1)){
            lastPacket = alertID;
            while(alerts.containsKey(lastPacket + 1))    lastPacket++;
        }
        alertSite.put(coord, alertID);
        setAlarm(alertID, end);
    }

    private void delayService(int[] coord, int dest) throws IOException {
        Set <int[]> k = alertSite.keySet();
        for (int[] i : k) {
            if (i[0] == coord[0] && i[1] == coord[1] ) {
                int a = alertSite.get(i);
                if (this.service.containsKey(dest))
                    if (this.service.get(dest) == a) return;
                this.service.put(dest, a);
                String info = "O mensagem de atraso, devido ao alerta "+ a +".";
                byte[] service = new packetWriter().service((byte) 2, dest, System.currentTimeMillis()/1000, info.getBytes());
                packet.get(0).push(c.encrypt(service, "vmw"));
                udp.send(c.encrypt(service, "car"));
                return;
            }
        }
    }

    public int newPacket(byte[] data){
        packetReader p = new packetReader(c.decrypt(data));
        int i = 0;
        switch (p.getType()){
            case 3:
                packet.get(0).push(c.encrypt(p.getPacket(), "vmw"));
                try {
                    data = c.encrypt(new packetWriter().ack(p.getHash()), "car");
                    udp.send(data);
                } catch (IOException e) {}
                break;
            case 4:
                i = p.getAlertID();
                if (i > lastPacket)
                    addAlert(i, p.getPriority(), p.getTimestamp() + p.getTTL(), p.getCoordAlert(), data);
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
                                delayService(p.getCoordService(), i);
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
        tcp.send(c.encrypt(new packetWriter().rsuID(ID), "vmw"));
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

        int id = Integer.parseInt(args[0]);//arg
        Thread host = new Thread(new Host_Comm(new TCP(new Socket("2001:690:2280:821:1000::10",8888)), packet, id, c));
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
