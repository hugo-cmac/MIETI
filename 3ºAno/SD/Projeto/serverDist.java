import java.io.*;
import java.net.*;
import java.net.Socket;
import java.util.*;
import java.util.concurrent.locks.*;

class User{
    Integer id;
    String nickname; 
    
    public User (Integer i){
        this.id = i;
    }

    public void setNickname(String n){
        this.nickname = n;
    }

    public String getNick(){
        return nickname;
    }
}

class Control {
    Lock l;
    HashMap<User, String> chat;
    Boolean b;
    int i;
    int fase;

    public Control( Lock lo, HashMap<User, String> c){
        this.l=lo;
        chat=c;
        this.b=false;
        this.i=0;
        this.fase=0;
    }

    public synchronized void Wait() throws InterruptedException {
        wait();
    }

    public synchronized void Notify() throws InterruptedException {
        notifyAll();
    }

    public void Lock() {
        l.lock();
    }

    public void Unlock(){
        l.unlock();
    }

    public void removeUser(User u){
        chat.remove(u);
    }

    public synchronized void Write( User u, String c) throws InterruptedException {
        try {
            if(chat.containsKey(u)){
                chat.remove(u);
            }
            chat.put( u, String.format("\n"+u.getNick()+c));
            this.b=true;
        } catch (Exception e) {}
        
    }

    public Map.Entry getMsg(PrintWriter out) throws InterruptedException{
        
        Iterator i = chat.entrySet().iterator();
        Map.Entry m = (Map.Entry) i.next();
        
        while(i.hasNext()){
            m = (Map.Entry) i.next();
        }
        return m;
    }

    public Boolean hasUpdated(){
        return this.b;
    }

    public synchronized void barrier() throws InterruptedException{
		int local=fase;
		i++;
		if(i==chat.size()){
			fase++;
            i=0;
            this.b=false;
			Notify();
		}else{
			while(local==fase){
				Wait();
			}
		}
	}
}

class ClientWriter implements Runnable{
    PrintWriter out;
    User u;
    Boolean on;
    Control c;

    public ClientWriter(PrintWriter o, User u, Boolean b, Control c) throws IOException{
        this.out=o;
        this.u=u;
        this.on=b;
        this.c=c;
    }
    public void run() {
        Map.Entry msg;
        try {  
            while(on){
                while(!c.hasUpdated()){
                    c.Wait();
                }

                c.barrier();
                
                msg = c.getMsg(out);
                if(!msg.getKey().equals(u)){
                    out.println(msg.getValue());
                }
            }
            out.close();

        } catch (Exception e) {}
    }
}

class Client implements Runnable{
    Socket s;
    Control c;
    User u;
    boolean on;

    public Client(Socket so, Control co, User us){
        this.s=so;
        this.c=co;
        this.u=us;
        this.on=true;
    }
    
	public void run() {
        String inBuff;

        try {
            BufferedReader in = new BufferedReader(new InputStreamReader(s.getInputStream()));
            PrintWriter out = new PrintWriter(s.getOutputStream(), true);

            newUser(in, out);

            StartT(out);
            out.print("\033[H\033[2J");   
            out.println("Chat opened - Welcome "+u.getNick());

            while ((inBuff = in.readLine()) != null) {
                SendS(2, inBuff);
            }

            SendS(3, null);

            in.close();
            out.close();
            s.close();
        } catch (IOException e) {}
    }
    
    public void StartT(PrintWriter o) throws IOException{
        new Thread(new ClientWriter(o, this.u, this.on, this.c)).start();
    }

    public void SendS(int opt, String msg){
        c.Lock();
        try {
            if(opt==3){
                c.removeUser(this.u);
            }
            c.Write(u, StringOption(opt, msg));
            c.Notify();
        } catch (Exception e) {
        } finally {
            c.Unlock();
        }
    }

    public String StringOption(int opt, String msg) {
        switch (opt) {
            case 1:
                return (String.format(" joined the chat."));
            case 2:
                return (String.format(": " + msg));
            case 3:
                return (String.format(" left the chat."));
        }
        return null;
    }

    public void newUser(BufferedReader in, PrintWriter out) throws IOException {
        String aws=new String();
        
        while (!aws.equals("y")) {
            out.println("Write your chat nickName: ");
            
            this.u.setNickname(in.readLine());
            
            out.println("(y-To continue/n-change)");
            
            aws = in.readLine();
        }
        SendS(1,null);
    }
}

class serverDist{
    public static void main(String[] args) throws IOException{
        ServerSocket ss = new ServerSocket(9999);
        Socket cs;
        Control c = new Control(new ReentrantLock(), new LinkedHashMap<User, String>());
        Integer id = 0;

        while(true){
            cs = ss.accept();
            Thread t = new Thread(new Client(cs, c, new User(id++)));
            t.start();
        }
    }
}
