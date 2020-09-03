import java.io.IOException;
import java.net.ServerSocket;
import java.net.SocketTimeoutException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Scanner;
import java.util.Random;
import java.io.FileWriter;
import java.io.PrintWriter;

class Packet {
    private HashMap<String, LinkedList<byte[]>> packet;
    private packetReader p;
    private IMTTDB db;
    private Crypto c;

    public Packet(IMTTDB db, Crypto c){
        this.packet = new HashMap<>();
        this.db = db;
        this.c = c;
    }

    private void broadCast(byte[] data){
        packet.forEach((key, value) ->  packet.get(key).push(c.encrypt(data, key.toLowerCase())));
    }

    private void registID(String id){
        packet.put(id, new LinkedList<>());
    }

    public void exit(String id){
        packet.remove(id);
    }

    private void addDNS(String registration, String ip) throws IOException {
      FileWriter file = new FileWriter("/etc/bind/db.auto.pt", true);
      PrintWriter writer = new PrintWriter(file);
      writer.println(registration + "  IN      AAAA    " + ip);
      writer.close();
    }

    public synchronized String newPacket(byte[] data, String id){
        if (data.length == 256)
            p = new packetReader(c.decrypt(data));
        else
            p = new packetReader(data);
        String i = null;
        switch (p.getType()){
            case 1:
                i = p.getBrandID();
                System.out.print("\u001b[s\u001b[1;40f\u001b[0K  {Adicionado HOST: "+i+"}\u001b[u");
                registID(i);
                try {
                    addDNS(i, p.getIP());
                } catch(IOException e) {}
                break;
            case 3:
                String registration = p.getRegistration();
                System.out.print("\u001b[s\u001b[1;40f\u001b[0K {Adicionado: "+id+" "+registration+"}\u001b[u");
                try {
                    addDNS(registration, p.getIP());
                } catch(IOException e) {}
                db.insertCar(registration, id, p.getModelOwner(), p.getDate());
                break;
            case 4:
                broadCast(data);
                break;
        }
        return i;
    }

    public boolean hasNewPacket(String id){
        if(packet.get(id).size() == 0)
            return false;
        return true;
    }

    public byte[] getPacket(String id){
        return packet.get(id).pop();
    }
}

class IMTT_Controller implements Runnable{
    private Packet packet;
    private Scanner in;
    private Boolean stay;
    private Boolean thread = false;
    private Integer id = 0;
    private IMTTDB db;

    public IMTT_Controller(Packet packet, IMTTDB db){
        this.in = new Scanner(System.in);
        this.packet = packet;
        this.stay = true;
        this.db = db;
    }
    private int coordX(int o){
        int c[] = {675, 925, 1165, 1405, 1645};
        return c[o];
    }

    private  int coordY(int o){
        int c[] = {40, 280, 520, 795};
        return c[o];
    }

    private void randomAlerts(){
        if (!thread) {
            thread = true;
            Thread t = new Thread(() -> {
                long init = 0;
                int ttl = 0, size, n = 0, x = 0, y = 0;
                synchronized (id) { size = id + 50; }
                Random r = new Random();
                String info;
                while(n < size){
                    synchronized (id) { n = ++id; }
                    info = "TESTE "+ n;
                    ttl = r.nextInt(50) + 10;
                    init = System.currentTimeMillis();
                    x = coordX(r.nextInt(5));
                    y = coordY(r.nextInt(4));
                    packet.newPacket( new packetWriter().alert( n, false,
                            init/1000, ttl, x, y, info.getBytes()), null);
                    System.out.print("\u001b[s\u001b[1;0fAlerta " + n + " enviado.\u001b[u");
                    db.insertAlert(n, false, init, ttl, x, y, info );
                    try {
                        Thread.sleep(r.nextInt(4) * 1000 + 1000);
                    } catch(Exception e) {}
                }
                System.out.print("\u001b[s\u001b[1;0f\u001b[0K\u001b[u");
                thread = false;
                return;
            });
            t.start();
        }
    }

    private void alert(){
        boolean prio = false;
        int n = 0;
        System.out.print("\u001b[3;0f\u001b[0JInformacao: ");
        String info = in.nextLine();
        System.out.print("Duracao: ");
        int ttl = in.nextInt();
        while (ttl > 65535 || ttl <= 0){
            System.out.print("Duracao invalida.\nDuracao: ");
            ttl = in.nextInt();
        }

        System.out.println("Cordenadas");
        System.out.print("X: ");
        int x = in.nextInt();
        while (x > 65535 && x < 0){
            System.out.print("Cordenada invalida.\nX: ");
            x = in.nextInt();
        }
        System.out.print("Y: ");
        int y = in.nextInt();
        while (y > 65535 && y < 0){
            System.out.print("Cordenada invalida.\nY: ");
            y = in.nextInt();
        }
        System.out.print("Prioritario: ");
        if (in.next().equals("sim"))    prio = true;
        synchronized (id) {
            n = ++id;
        }
        long init = System.currentTimeMillis();
        packet.newPacket( new packetWriter().alert( n, prio, init/1000, ttl, x, y, info.getBytes()), null);
        db.insertAlert(n, prio, init, ttl, x, y, info );
        System.out.print("\u001b[1;0fAlerta " + n + " enviado.");
    }

    private void consult(){
        boolean s = true;
        while (s){
            //INTERFACE
            System.out.println("\u001b[3;0f\u001b[0J1 - Alertas\n2 - Carros\n3 - Ver alerta\n4 - Ver carro\n0 - Sair");
            switch (in.nextInt()){
                case 1:
                    in.nextLine();
                    System.out.println("\u001b[3;0f\u001b[0J");
                    db.printAlerts();
                    in.nextLine();
                    break;
                case 2:
                    in.nextLine();
                    System.out.println("\u001b[3;0f\u001b[0J");
                    db.printCars();
                    in.nextLine();
                    break;
                case 3:
                    in.nextLine();
                    System.out.print("\u001b[3;0f\u001b[0JID: ");
                    int i = in.nextInt();
                    in.nextLine();
                    System.out.println("\u001b[3;0f\u001b[0J");
                    db.printAlert(i);
                    in.nextLine();
                    break;
                case 4:
                    in.nextLine();
                    System.out.print("\u001b[3;0f\u001b[0JMatricula: ");
                    String r = in.nextLine();
                    System.out.println("\u001b[3;0f\u001b[0J");
                    db.printCar(r);
                    in.nextLine();
                    break;
                case 0:
                    s = false;
                    break;
                default:
                    System.out.println("Opcao invalida");
                    break;
            }
        }
    }

    public void run() {
        System.out.print("\u001b[2J\u001b[2;0f");
        while (stay){
            System.out.println("----------------------------------------------------------------------------");
            System.out.println("\u001b[3;0f\u001b[0J1 - Alerta\n2 - Envio aleatorio de alertas\n3 - Consultar BD\n0 - Sair");
            switch (in.nextInt()){
                case 1:
                    in.nextLine();
                    alert();
                    break;
                case 2:
                    in.nextLine();
                    randomAlerts();
                    break;
                case 3:
                    in.nextLine();
                    consult();
                    break;
                case 0:
                    //sair
                    System.exit(0);
                    break;
                default:
                    System.out.println("Opcao invalida");
                    break;
            }
        }
    }
}

class Host_Comm implements Runnable{
    private TCP tcp;
    private Packet packet;
    private String ID;

    public Host_Comm(TCP tcp, Packet packet){
        this.tcp = tcp;
        this.packet = packet;
        this.ID = null;
    }
    private void getID() throws IOException{
        while (ID == null){
            try {
                byte[] data = tcp.read();
                ID = packet.newPacket(data, null);
            } catch(SocketTimeoutException e) {}
        }
    }
    public void run() {
        byte[] data;
        try {
            getID();
            while (tcp.isOn()){
                try{
                    data = tcp.read();
                    if (tcp.isOn())   packet.newPacket(data, ID);
                }catch (SocketTimeoutException e) {
                    if (packet.hasNewPacket(ID)) {
                        data = packet.getPacket(ID);
                        tcp.send(data);
                    }
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        packet.exit(ID);
        return;
    }
}
public class IMTT {
    private static void initDNS() throws InterruptedException, IOException {
        String[] command = {".././dns", "start"};
        Process pid = Runtime.getRuntime().exec(command);
        pid.waitFor();
    }
    public static void main(String[] args) throws IOException, InterruptedException {
        IMTTDB db = new IMTTDB();
        db.connect();
        Crypto c = new Crypto("imtt", "1234");
        Packet packet = new Packet(db, c);
        ServerSocket ss = new ServerSocket(9999);

        initDNS();

        Thread manager = new Thread(new IMTT_Controller(packet, db));
        manager.start();

        Thread host;
        while (true) {
            host = new Thread(new Host_Comm(new TCP(ss.accept()), packet));
            host.start();
        }
    }
}
