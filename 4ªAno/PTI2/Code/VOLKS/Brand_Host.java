import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.*;

class Packet {
    private HashMap<Integer, LinkedList<byte[]>> packet;
    private HashMap<Integer, String> cars;
    private packetReader p;
    private BrandDB db;
    private Crypto c;

    public Packet(BrandDB db, Crypto c){
        this.packet = new HashMap<>();
        this.cars = new HashMap<>();
        this.db = db;
        this.c = c;
    }

    private void broadCast(byte[] data){
        packet.forEach((key, value) -> packet.get(key).push(data));
        packet.get(0).remove(data);
    }

    public void registID(int id){
        packet.put(id, new LinkedList<>());
    }

    public Set<Integer> getCars(){
        return cars.keySet();
    }

    public String getRegistration(int h){
      return cars.get(h);
    }

    public void exit(int id){
      packet.remove(id);
    }

    public synchronized int newPacket(byte[] data){
        if (data.length == 256) p = new packetReader(c.decrypt(data));
        else p = new packetReader(data);
        int i = 0;
        switch (p.getType()){
            case 2:
                i = p.getRsuID();
                System.out.print("\u001b[s\u001b[1;40f\u001b[2K  {Adicionado: RSU "+ i +"}\u001b[u");
                registID(i);
                break;
            case 3:
                String registration = p.getRegistration();
                i = p.getHash();
                if (!cars.containsKey(i)) {
                    cars.put(p.getHash(), registration);
                    System.out.print("\u001b[s\u001b[1;40f\u001b[2K  {Adicionado: "+registration+"}\u001b[u");
                    packet.get(0).push(c.encrypt(p.getPacket(), "imtt"));
                    db.insertCar(registration, p.getModelOwner(), p.getDate());
                }
                break;
            case 4:
                broadCast(c.encrypt(p.getPacket(), "car"));
                i = p.getAlertID();
                System.out.print("\u001b[s\u001b[2;40f\u001b[2K  {Alerta: "+i+"}\u001b[u");
                db.insertEvent(i, true, p.getPriority(), "",
                               p.getTimestamp()*1000, p.getTTL(), p.getCoordXAlert(),
                               p.getCoordYAlert(), p.getInfo());;
                break;
            case 7:
                broadCast(c.encrypt(data, "car"));
                break;
            case 8:
                i = p.getDest();
                System.out.print("\u001b[s\u001b[1;0f\u001b[2K"+ cars.get(i) + ": " +p.getServiceInfo()+"\u001b[2;40f\u001b[1K\u001b[u");
                db.insertEvent(p.getServiceID(), false, false,cars.get(i), p.getServiceTimestamp()*1000,
                               0, 0, 0, p.getServiceInfo());
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

class Brand_Controller implements Runnable{
    private Packet packet;
    private Scanner in;
    private Boolean stay;
    private String BrandID;
    private BrandDB db;
    private Boolean thread = false;

    public Brand_Controller(Packet packet, String brandID, BrandDB db){
        this.in = new Scanner(System.in);
        this.packet = packet;
        this.stay = true;
        this.BrandID = brandID;
        this.db = db;
    }

    public void startService(){
        if (!thread) {
            thread = true;
            Thread t = new Thread(() -> {
                Random r = new Random();
                int i = 0;
                long init = 0;
                String info;
                while (i != 20) {
                    info = "TrapSol:  Gasoleo: "  + (float) (r.nextInt(5) + 10) / 10 + " €/l" +
                         "\n          Gasolina: " + (float) (r.nextInt(5) + 10) / 10 + " €/l";
                    Set<Integer> cars = packet.getCars();
                    init = System.currentTimeMillis();
                    for (Integer d : cars) {
                        packet.newPacket(new packetWriter().service((byte) 1, d,
                                (init/1000), info.getBytes()));
                        db.insertEvent(1, false, false, packet.getRegistration(d), init, 0, 0, 0, info);
                    }
                    System.out.print("\u001b[s\u001b[1;0f\u001b[2K" + info + "\u001b[u");
                    try {
                        Thread.sleep(r.nextInt(4) * 1000 + 10000);
                        i++;
                    } catch (Exception e) {
                    }
                }
                System.out.print("\u001b[s\u001b[1;0f\u001b[0K\u001b[2;0f\u001b[0K\u001b[u");
                thread = false;
                return;
            });
            t.start();
        }
    }

    private void consult(){
        boolean s = true;
        while (s){
            //INTERFACE
            System.out.println("\u001b[4;0f\u001b[0J1 - Todos os eventos\n2 - Todos os alertas\n3 - Todos os servicos\n4 - Todos os Carros\n5 - Ver evento\n6 - Ver carro\n0 - Sair");
            switch (in.nextInt()){
                case 1:
                    in.nextLine();
                    System.out.println("\u001b[4;0f\u001b[0J");
                    db.printAll();
                    in.nextLine();
                    break;
                case 2:
                    in.nextLine();
                    System.out.println("\u001b[4;0f\u001b[0J");
                    db.printAlerts();
                    in.nextLine();
                    break;
                case 3:
                    in.nextLine();
                    System.out.println("\u001b[4;0f\u001b[0J");
                    db.printServices();
                    in.nextLine();
                    break;
                case 4:
                    in.nextLine();
                    System.out.println("\u001b[4;0f\u001b[0J");
                    db.printCars();
                    in.nextLine();
                    break;
                case 5:
                    in.nextLine();
                    System.out.print("\u001b[4;0f\u001b[0JAlert: ");
                    boolean a = false;
                    if (in.next().equals("sim"))    a = true;
                    in.nextLine();
                    System.out.print("ID do Evento: ");
                    int i = in.nextInt();
                    in.nextLine();
                    System.out.println("\u001b[4;0f\u001b[0J");
                    db.printEvent(i, a);
                    in.nextLine();
                    break;
                case 6:
                    in.nextLine();
                    System.out.print("\u001b[4;0f\u001b[0JMatricula: ");
                    String r = in.nextLine();
                    System.out.println("\u001b[4;0f\u001b[0J");
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
        System.out.print("\u001b[2J");
        while (stay){
            System.out.println("\u001b[3;0f----------------------------------------------------------------------------");
            System.out.println("\u001b[4;0f\u001b[0J1 - Iniciar Servico\n2 - Consultar BD\n0 - Sair");
            switch (in.nextInt()){
                case 1:
                    in.nextLine();
                    startService();
                    break;
                case 2:
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

class IMTT_Comm implements Runnable{
    private TCP tcp;
    private Packet packet;
    private Crypto c;
    private int ID;
    private String brand;

    public IMTT_Comm(TCP tcp, Packet packet, Crypto c, String brand){
        this.tcp = tcp;
        this.packet = packet;
        this.c = c;
        this.ID = 0;
        this.brand = brand;
    }

    private void sendID() throws IOException{
        tcp.send(c.encrypt(new packetWriter().brandID(brand.getBytes(), tcp.getIP()), "imtt"));
    }

    public void run() {
        byte[] data;
        packet.registID(ID);
        try {
            sendID();
            while (tcp.isOn()){
                try{
                    data = tcp.read();
                    if (tcp.isOn())   packet.newPacket(data);
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

class RSU_Comm implements Runnable{
    private TCP tcp;
    private Packet packet;
    private int ID;

    public RSU_Comm(TCP tcp, Packet packet){
        this.tcp = tcp;
        this.packet = packet;
        this.ID = 0;
    }

    private void getID() throws IOException{
        while (ID == 0){
            try {
                byte[] data = tcp.read();
                ID = packet.newPacket(data);
            }catch (SocketTimeoutException e){}
        }
    }
    public void run() {
        byte[] data;
        try {
            getID();
            while (tcp.isOn()){
                try{
                    data = tcp.read();
                    if (tcp.isOn())   packet.newPacket(data);
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

public class Brand_Host {

    public static void main(String[] args) throws IOException {
        String id = "VOLKS";
        BrandDB db = new BrandDB(id);
        db.connect();
        Crypto c = new Crypto(id.toLowerCase(), "1234");
        Packet packet = new Packet(db, c);

        Thread imtt = new Thread(new IMTT_Comm(new TCP(new Socket("2001:690:2280:820:1000::20",9999)), packet, c, id));
        imtt.start();

        Thread controller = new Thread(new Brand_Controller(packet, id, db));
        controller.start();

        ServerSocket ss = new ServerSocket(8888);
        Thread rsu;
        while (true) {
            rsu = new Thread(new RSU_Comm(new TCP(ss.accept()), packet));
            rsu.start();
        }
    }
}
