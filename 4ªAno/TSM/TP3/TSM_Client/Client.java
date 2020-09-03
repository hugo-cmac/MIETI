import javax.sound.sampled.*;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.*;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;



class Player{
    private static final int MAX = 100;
    //Packet control
    private int[] missing;
    private int last = MAX;
    private int dataSize;
    private boolean cut = false;
    //Decompresser handling
    private ConcurrentHashMap<Integer, byte[]> compressed;
    private byte[] buffer;
    private int index = 0;
    private int pos = 0;
    private int left = 8;
    //Music playback
    private AudioFormat audio;
    private SourceDataLine sourceLine;
    private boolean play = false;
    private boolean end = true;
    //print
    private String name;
    private int div = 0;

    public Player(){
        missing = new int[1500];
        compressed = new ConcurrentHashMap<>();
    }

    private void printAdvice(String advice){
        System.out.print("\u001b[s\u001b[2;39f\u001b[0K >> ["+advice+"]\u001b[u");
    }

    private void printSong(int time){
        int min = time/60;
        int sec = time - min*60;
        System.out.print("\u001b[s\u001b[1;0f\u001b[0K Musica: \""+name+"\"    Tempo restante: "+ min +"m"+ sec +"s\u001b[u");
    }

    private void clear(){
        System.out.print("\u001b[s\u001b[1;0f\u001b[0K\u001b[u");
    }

    public boolean isOn(){
        return play;
    }

    public boolean hasEnded(){
        return end;
    }

    private void begin(){
        index = 0;
        pos = 0;
        left = 8;
    }

    public void infoPacket(byte[] packet) throws LineUnavailableException {
        int channels =      (((int) packet[2] & 0xff)<<8)   | (((int) packet[1] & 0xff));
        int sampleRate =    (((int) packet[6] & 0xff)<<24)  | (((int) packet[5] & 0xff)<<16) |
                            (((int) packet[4] & 0xff)<<8)   | (((int) packet[3] & 0xff));
        int bits =          (((int) packet[8] & 0xff)<<8)   | ((int) packet[7] & 0xff);
        dataSize =          (((int) packet[12] & 0xff)<<24) | (((int) packet[11] & 0xff)<<16) |
                            (((int) packet[10] & 0xff)<<8)  | (((int) packet[9] & 0xff));

        div = (sampleRate*channels*bits)/8;

        audio = new AudioFormat(sampleRate, bits, channels, true, false);
        sourceLine = AudioSystem.getSourceDataLine(audio);
        sourceLine.open(audio, 4096);

        sourceLine.start();
        play = true;
        end = false;
        last = MAX;
        int i = 0;
        while (i < 1500) missing[i++] = 1;
        compressed.clear();
        begin();
    }

    private void printBytes(byte[] p, int size){
        for (int i = 0; i < size; i++){
            System.out.format("%02X.", p[i]);
        }
        System.out.println();
    }

    public void add(int i, byte[] data, boolean last, int size){
        if(missing[i] == 1){
            missing[i] = 0;
            byte[] temp = new byte[size];
            try {
                if (last)
                    System.arraycopy(data, 2, temp, 0, size-2);
                else
                    System.arraycopy(data, 2, temp, 0, size);
            }catch (ArrayIndexOutOfBoundsException a){
                end = true;
                play = false;
                dataSize = -1;
                //printAdvice("Erro na reproducao");
                printAdvice("Size: "+ size+"  i: "+i+"  last: " + last);
                return;
            }
            compressed.put(i, temp);
        }
    }

    public synchronized boolean last(){
        //System.out.println((index+1) + " < " + last);
        while ((index+1 < last) && !end) {
            try {
                wait();
            } catch (InterruptedException e) {}
        }
        int i = 0;
        while (i < 1500) missing[i++] = 1;
        last = MAX;
        return false;
    }

    public byte[] getMissing(int seq){
        byte[] packet = new byte[1472];
        int p = 1, i = 0;
        boolean l = false;

        if (seq == 0) seq = last;
        else l = true;

        packet[0] = (byte) (3<<6);

        while (i <= seq){
            if (missing[i] == 1){
                packet[p++] = (byte) (i>>8);
                packet[p++] = (byte) i;
            }
            i++;
        }
        packet[p++] = (byte) (1500>>8);
        packet[p++] = (byte) 1500;

        if (!l){
            last += MAX;
            if (p != 3) last -= (p>>2 - 1);
        } else last = seq;
        return packet;
    }

    private synchronized void inc() throws InterruptedException {
        pos++;
        if (cut){
            cut = false;
            dataSize -= 4096;
            printSong(dataSize/div);
            if (dataSize <= 0) throw new  InterruptedException();
        }
        if (pos == buffer.length){
            pos = 0;
            index++;
            if (index+1 == last) notify();
            while (!compressed.containsKey(index))
                Thread.sleep(50);
            buffer = compressed.remove(index);
        }
    }

    private float getBits(boolean compl, boolean decim, int size) throws InterruptedException {
        int mask = 255;
        short n = 0;
        float f;

        if (compl){
            left--;
            size--;
            if (((buffer[pos]>>left) & 1) != 1) compl = false;
            if (left == 0){
                left = 8;
                inc();
            }
        }
        if (decim){
            left--;
            size--;
            if (((buffer[pos]>>left) & 1) != 1) decim = false;
            if (left == 0){
                left = 8;
                inc();
            }
        }
        while (left <= size){
            size -= left;
            n |= ((((short) buffer[pos] & (mask >> (8-left)))) << size);
            left = 8;
            inc();
        }
        if (size > 0){
            mask >>= (8 - left);
            left -= size;
            n |= ((short) buffer[pos] & mask) >> left;
        }

        if (decim) f = (float) ((float) n + 0.5);
        else f = n;
        if (compl) f = -f;
        return f;
    }

    public synchronized void stop(){
        if (!end) {
            end = true;
            play = false;
            dataSize = -1;
            notify();
        }else{
            printAdvice("Nenhum som a tocar");
        }
    }

    public synchronized void pause(){
        if (play) play = false;
        else play = true;
    }

    public void play(String name){
        byte[] data = new byte[4096];
        this.name = name;

        short   left, right;
        int     minAvg, minOff,
                maxAvg, maxOff,
                size, i, j;
        float   avg, off,
                davg, doff;

        try {
            while (true){
                while (!compressed.containsKey(0));
                buffer = compressed.remove(0);

                size = 0;
                while(size < 2097152){
                    minAvg = (int) getBits(false, false, 5);
                    minOff = (int) getBits(false, false, 5);
                    maxAvg = (int) getBits(false, false, 5);
                    maxOff = (int) getBits(false, false, 5);

                    avg = off = 0;
                    i = j = 0;
                    while (i < 1024){
                        if (getBits(false,false, 1) == 0){
                            davg = getBits(true, true, minAvg);
                            doff = getBits(true, true, minOff);
                        }else{
                            davg = getBits(true, true, maxAvg);
                            doff = getBits(true, true, maxOff);
                        }

                        avg = davg + avg;
                        off = doff + off;

                        left =  (short) (avg + off);
                        right = (short) (avg*2 - left);

                        data[j++] = (byte) ((left) & 0xff);
                        data[j++] = (byte) ((left >> 8) & 0xff);
                        data[j++] = (byte) ((right) & 0xff);
                        data[j++] = (byte) ((right >> 8) & 0xff);

                        i++;
                    }
                    cut = true;
                    size += j;
                    while (end) return;
                    while (!play || end) Thread.sleep(200);

                    sourceLine.write(data, 0, j);
                }
                begin();
            }
        }catch (InterruptedException e){
            sourceLine.drain();
            sourceLine.close();
            end = true;
            play = false;
            div = 0;
            this.name = null;
            clear();
            return;
        }
    }
}
class UDP {
    private int port;
    private InetAddress multicast;
    private MulticastSocket socket;
    private int timeout = 500;
    private int size;

    public UDP(int port, String ip) throws IOException {
        multicast = InetAddress.getByName(ip);
        this.port = port;
        socket = new MulticastSocket(this.port);
        socket.joinGroup(multicast);
        socket.setLoopbackMode(false);
        socket.setTimeToLive(1);
        socket.setReceiveBufferSize(294400);
        socket.setSoTimeout(timeout);
    }

    public synchronized void send(byte[] data) throws IOException{
        DatagramPacket packet = new DatagramPacket(data, data.length, this.multicast, 9999);
        socket.send(packet);
    }

    public synchronized byte[] read() throws IOException, SocketTimeoutException{
        byte[] data = new byte[1472];
        DatagramPacket packet = new DatagramPacket(data,1472);
        socket.receive(packet);
        size = packet.getLength();
        return data;
    }

    public synchronized byte[] readNoTimeout() throws IOException{
        socket.setSoTimeout(0);
        byte[] data = new byte[1472];
        DatagramPacket packet = new DatagramPacket(data,1472);
        socket.receive(packet);
        size = packet.getLength();
        socket.setSoTimeout(timeout);
        return data;
    }

    public int getSize(){
        return size;
    }
}

class TCP{
    private int port;
    private Socket socket;
    private boolean connection;
    private int size = 0;

    public TCP(int port, String address) throws IOException {
        this.port = port;
        socket = new Socket(address, port);
        connection = true;
    }

    public void send(byte[] data) throws IOException{
        DataOutputStream packet = new DataOutputStream(socket.getOutputStream());
        packet.write(data);
    }

    public byte[] read() throws IOException {
        byte[] data = new byte[10000];
        DataInputStream packet = new DataInputStream(socket.getInputStream());
        size = packet.read(data);
        if (size < 1)
            connection = false;
        return data;
    }

    public boolean isOn(){
        return connection;
    }

    public void close() throws IOException{ socket.close(); }
}

class StreamPlayer implements Runnable{
    private LinkedList<String> playlist;
    private LinkedList<String> names;
    private UDP udp;
    private TCP tcp;
    private String ip;
    private Player p;
    private byte[] packet;
    private int sequence = 0;
    private boolean last = false;

    public StreamPlayer(Player p, String ip, TCP tcp){
        playlist = new LinkedList<>();
        names = new LinkedList<>();
        this.p = p;
        this.ip = ip;
        this.tcp = tcp;
    }

    private static void printAdvice(String advice){
        System.out.print("\u001b[s\u001b[2;39f\u001b[0K >> ["+advice+"]\u001b[u");
    }

    public void printPlaylist(){
        ListIterator l = names.listIterator();
        System.out.println("\u001b[3;0f\u001b[0J+-------------------Playlist-------------------+");
        while (l.hasNext())
            System.out.format("| %-40s     |\n", l.next());
        System.out.println("+----------------------------------------------+");
        getLine();
    }

    private int getType(byte[] packet){
        return (((int) packet[0] & 0xff)>>6);
    }

    private int getSequence(byte[] packet){
        return (((int) packet[0] & 0x07)<<8) + ((int) packet[1] & 0xff);
    }

    private void getLine(){
        try {
            System.in.read();
        } catch (IOException ioException) {
            printAdvice("IO");
        }
    }

    public boolean isPlaylistEmpty(){
        if (p.hasEnded())
            return playlist.isEmpty();
        return false;
    }

    public void clearPlaylist(){
        names.clear();
        playlist.clear();
    }

    public void addPlaylist(String path, String name){
        playlist.add(path);
        names.add(name);
        if (p.hasEnded()) sendNext();
    }

    private void sendNext(){
        String path = playlist.pop();
        byte[] packet = new byte[path.length()+10];
        packet[0] = 1;
        System.arraycopy(path.getBytes(), 0, packet, 1, path.length());
        try {
            tcp.send(packet);
        } catch (IOException e) {
            printAdvice("Erro no Play");
        }
    }

    private void connect(){
        while (true){
            try {
                Thread.sleep(500);
                udp = new UDP(8888, ip);
                break;
            } catch (IOException e) {
                printAdvice("Sem ligacao");
            } catch (InterruptedException e) {}
        }
    }

    public void Start(){
        String n = names.pop();
        Thread worker = new Thread(() ->{
            p.play(n);
            return;
        });
        worker.start();
    }

    public void run(){
        connect();
        while (true){
            try {
                while (!p.isOn()){
                    if (!playlist.isEmpty())
                        sendNext();
                    while (true){
                        packet = udp.readNoTimeout();
                        if (getType(packet) == 0){
                            p.infoPacket(packet);
                            Start();
                            break;
                        }
                    }
                }
                while (!p.hasEnded()){
                    try {
                        packet = udp.read();
                        switch (getType(packet)){
                            case 1:
                                sequence = getSequence(packet);
                                p.add(sequence, packet, false, udp.getSize()-2);
                                break;

                            case 2:
                                last = true;
                                sequence = getSequence(packet);
                                p.add(sequence, packet, true, udp.getSize());
                                throw new SocketTimeoutException();
                        }
                    }catch (SocketTimeoutException t){
                        if (!p.hasEnded()) {
                            if (last) {
                                udp.send(p.getMissing(sequence));
                                last = p.last();
                            } else {
                                udp.send(p.getMissing(0));
                            }
                        }
                    }
                }
            } catch (IOException e) {
                connect();
            } catch (LineUnavailableException l){
                printAdvice("Placa de som em uso");
            }
        }
    }
}

class MusicManagment{
    private HashMap <String, LinkedList<String>> artPerGenre;
    private HashMap <String, LinkedList<String>> albPerArtis;
    private HashMap <String, LinkedList<String>> musPerAlbum;
    private HashMap <String, String> pathSongs;

    public void print(){
        for (String name: musPerAlbum.keySet()){
            String key = name;
            String value = musPerAlbum.get(name).toString();
            System.out.println(key + " " + value);
}
    }

    public MusicManagment(){
        artPerGenre = new HashMap<>();
        albPerArtis = new HashMap<>();
        musPerAlbum = new HashMap<>();
        pathSongs = new HashMap<>();
    }

    public void addSong(String path){
        int i;
        String[] item = path.split("/");
        String music = item[4].substring(0, item[4].lastIndexOf("."));
        if (!pathSongs.containsKey(music)){ //existe a musica
            i = 1;
            if (artPerGenre.containsKey(item[1])){ //existe o genero
                i = 2;
                if (albPerArtis.containsKey(item[2])) { //existe o artista
                    i = 3;
                    if (musPerAlbum.containsKey(item[3])){ //existe o album
                        musPerAlbum.get(item[3]).add(music);
                        pathSongs.put(music, path);
                        i = 4;
                    }
                }
            }
            if (i < 2) {
                artPerGenre.put(item[1], new LinkedList<>());
                artPerGenre.get(item[1]).add(item[2]);
            }
            if (i < 3){
                albPerArtis.put(item[2], new LinkedList<>());
                albPerArtis.get(item[2]).add(item[3]);
            }
            if (i < 4){
                musPerAlbum.put(item[3], new LinkedList<>());
                musPerAlbum.get(item[3]).add(music);
                pathSongs.put(music, path);
            }
        }
    }

    public void updateList(String list){
        int i = 0;
        String[] musics = list.split("\n");
        while (i < musics.length-1) {
            addSong(musics[i++]);
        }
    }

    public Set<String> getGenres(){
        return artPerGenre.keySet();
    }

    public Set<String> getArts(){
        return albPerArtis.keySet();
    }

    public Set<String> getAlbs(){
        return musPerAlbum.keySet();
    }

    public LinkedList<String> getArtsFrom(String genre){
        return artPerGenre.get(genre);
    }

    public LinkedList<String> getAlbsFrom(String art){
        return albPerArtis.get(art);
    }

    public LinkedList<String> getMusicsFrom(String alb){
        return musPerAlbum.get(alb);
    }

    public String getPath(String music){
        return pathSongs.get(music);
    }

    public Set<String> getAll(){
        return pathSongs.keySet();
    }

    public boolean isEmpty(){
        return pathSongs.isEmpty();
    }
}

public class Client {
    private static Scanner in = new Scanner(System.in);
    private static MusicManagment m;
    private static Player p;
    private static StreamPlayer s;
    private static TCP tcp;

    private static void printAdvice(String advice){
        System.out.print("\u001b[s\u001b[2;39f\u001b[0K >> ["+advice+"]\u001b[s");
    }
    private static void clearScreen(){
        System.out.print("\u001b[0;0f\u001b[2J");
    }
    private static char getChar(){
        char op;
        while (true){
            try {
                op = in.next().toLowerCase().charAt(0);
                return op;
            } catch (InputMismatchException i){
                printAdvice("Input invalido");
            }
        }
    }
    private static int getInt(){
        int op;
        while (true){
            try {
                op = in.nextInt();
                return op;
            } catch (InputMismatchException i){
                printAdvice("Input invalido");
            }
        }
    }
    private static String printSet(Set<String> temp0, String categorie){
        int i = 0;
        String[] temp1 = new String[temp0.size()];
        System.out.println("\u001b[3;0f\u001b[0J+--------------------"+ categorie +"--------------------+");
        for (String t: temp0) {
            System.out.format("| %-2d:  %-40s|\n", (i+1), t);
            temp1[i++] = t;
        }
        System.out.println("| 0 :  Abandonar                               |");
        System.out.println("+----------------------------------------------+");
        System.out.print(" -> ");
        i = getInt();
        clearScreen();
        if (i == 0)
            return null;
        return temp1[i-1];
    }
    private static String printArray(LinkedList<String> temp, String categorie){
        int i = -1;
        System.out.println("\u001b[3;0f\u001b[0J+--------------------"+ categorie +"--------------------+");
        while (++i < temp.size()){
            System.out.format("| %-2d: %-40s |\n", (i+1), temp.get(i));
        }
        System.out.println("| 0 : Abandonar                                |");
        System.out.println("+----------------------------------------------+");
        System.out.print(" -> ");
        i = getInt();
        clearScreen();
        if (i == 0)
            return null;
        return temp.get(i-1);
    }

    private static void update(){
        byte[] packet = new byte[4];
        packet[0] = 0;
        try {
            tcp.send(packet);
            packet = tcp.read();
        } catch (IOException e) {
            printAdvice("Erro na atualizacao");
        }
        m.updateList(new String(packet));
    }

    private static void control(int opcode){
        byte[] packet = new byte[4];
        packet[0] = (byte) opcode;
        try {
            tcp.send(packet);
        } catch (IOException e) {
            printAdvice("Erro no Controlo");
        }
    }

    private static void menu(){
        char op = 'x';
        String temp;
        while (op != 'e'){
            System.out.println("\u001b[2;0f< [P]lay/Pause > < [N]ext > < [S]top >\u001b[3;0f+---------------------Menu---------------------+");
            System.out.println("| G: Ver Generos                               |");
            System.out.println("| I: Ver Interprete                            |");
            System.out.println("| A: Ver Albuns                                |");
            System.out.println("| M: Ver Musicas                               |");
            System.out.println("| L: Ver Playlist                              |");
            System.out.println("| R: Atualizar lista                           |");
            System.out.println("| E: Abandonar                                 |");
            System.out.println("+----------------------------------------------+");
            System.out.print(" -> \u001b[0K");
            op = getChar();
            switch (op){
                case 'p':
                    //play/pause
                    if (!p.hasEnded())
                        p.pause();
                    else
                        printAdvice("Nenhum som a tocar");
                    break;
                case 'n':
                    //seguinte
                    p.stop();
                    control(2);
                    if (s.isPlaylistEmpty())
                        printAdvice("Playlist vazia");
                    break;
                case 's':
                    //stop
                    s.clearPlaylist();
                    p.stop();
                    control(2);
                    break;
                case 'g':
                    //generos
                    temp = printSet(m.getGenres(), "Genero");
                    if (temp != null){
                        temp = printArray(m.getArtsFrom(temp), "Artist");
                        if (temp != null){
                            temp = printArray(m.getAlbsFrom(temp), "Albuns");
                            if (temp != null){
                                temp = printArray(m.getMusicsFrom(temp), "Musica");
                                if (temp != null) {//toca musica
                                    s.addPlaylist(m.getPath(temp), temp);
                                }
                            }
                        }
                    }
                    break;
                case 'i':
                    //Artista
                    temp = printSet(m.getArts(), "Artist");
                    if (temp != null){
                        temp = printArray(m.getAlbsFrom(temp), "Albuns");
                        if (temp != null){
                            temp = printArray(m.getMusicsFrom(temp), "Musica");
                            if (temp != null)//toca musica
                                s.addPlaylist(m.getPath(temp), temp);
                        }
                    }
                    break;
                case 'a':
                    //album
                    temp = printSet(m.getAlbs(), "Albuns");
                    if (temp != null){
                        temp = printArray(m.getMusicsFrom(temp), "Musica");
                        if (temp != null)//toca musica
                            s.addPlaylist(m.getPath(temp), temp);
                    }
                    break;
                case 'm':
                    //musicas
                     temp = printSet(m.getAll(), "Musica");
                      if (temp != null)//toca musica
                          s.addPlaylist(m.getPath(temp), temp);
                    break;
                case 'l':
                    //playlist
                    s.printPlaylist();
                    clearScreen();
                    break;
                case 'r':
                    //atualizar
                    update();
                    break;
                case 'e':
                    //sair
                    clearScreen();
                    System.out.println("Bye");
                    System.exit(0);
                    break;
                default:
                    printAdvice("Opcao invalida");
                    break;
            }
        }
    }

    public static void main(String[] args) throws IOException {
        clearScreen();
        p = new Player();
        m = new MusicManagment();
        tcp = new TCP(9999, "127.0.0.1");
        byte[] packet = tcp.read();
        s = new StreamPlayer(p, new String(packet), tcp);
        Thread t = new Thread(s);
        t.start();
        update();
        menu();
    }
}


