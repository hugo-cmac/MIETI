package com.Tuber.java;

import java.io.*;
import java.io.FileOutputStream;
import java.time.LocalDate;
import java.util.*;

public class Empresa {
    private HashMap<Integer, Servico> listaServ = new HashMap<>();
    private HashMap<String, Veiculo> listaVeic = new HashMap<>();
    private HashMap<Integer, Cliente> listaClie = new HashMap<>();
    private Scanner buf = new Scanner(System.in);
    private int nC=1000;
    private int nS=2000;

    public Empresa() {}

    public void novoCliente(){
        try{
            System.out.print("Nome: ");
            buf=new Scanner(System.in);
            String nome=buf.nextLine();
            for (Integer k: listaClie.keySet()){
                while (nome.equals(listaClie.get(k).getNome())){
                    System.out.println("Nome existente.");
                    nome=buf.nextLine();
                }
            }
            System.out.print("NIF: ");
            buf=new Scanner(System.in);
            int nif = buf.nextInt();
            for (Integer k: listaClie.keySet()){
                while (nif==listaClie.get(k).getNIF()){
                    System.out.println("NIF existente.");
                    nif=buf.nextInt();
                }
            }
            nC++;
            System.out.println("Cliente adicionado com Sucesso.");
            Cliente c = new Cliente(nC,nome,nif);
            listaClie.put(nC,c);
        }catch (Exception e){
            System.out.println("Erro do sistema.\nTente novamente.\n");
        }
    }

    public void novoVeiculo(){
        try{
            System.out.println("Matrícula: ");
            buf=new Scanner(System.in);
            String m = buf.nextLine();
            for (String k : listaVeic.keySet()){
                while (m.equals(k)){
                    System.out.print("Matrícula existente.");
                    buf=new Scanner(System.in);
                    m = buf.nextLine();
                }
            }
            System.out.println("Tipo de veículo:\n1-Ligeiro.\n2-Mercadorias.");
            int op = buf.nextInt();
            int cap=0;
            while (op!=-1){
                switch (op){
                    case 1:
                        System.out.println("Capacidade do veículo:\n1-4 Pessoas.\n2-6 Pessoas.\n3-8 Pessoas.");
                        while (op!=-1){
                            op = buf.nextInt();
                            switch (op){
                                case 1:cap=4;op=-1;break;
                                case 2:cap=6;op=-1;break;
                                case 3:cap=8;op=-1;break;
                                default:
                                    System.out.println("Opção inválida.");
                                    break;
                            }
                        }
                        Veiculo vp = new VPessoas(m,"Cede - Tuber",true,0.0,cap);
                        listaVeic.put(m,vp);
                        break;
                    case 2:
                        System.out.println("Capacidade do veículo:\n1-350kg.\n2-15000kg.");
                        while (op!=-1){
                            op = buf.nextInt();
                            switch (op){
                                case 1:cap=350;op=-1;break;
                                case 2:cap=15000;op=-1;break;
                                default:
                                    System.out.println("Opção inválida.");
                                    break;
                            }
                        }
                        Veiculo vm = new VMercadorias(m,"Cede - Tuber",true,0.0,cap);
                        listaVeic.put(m,vm);
                        break;
                    default:
                        System.out.println("Opção inválida.");
                        break;
                }
            }
        }catch (Exception e){
            System.out.println("Erro do sistema.\nTente novamente.\n");
        }
    }

    public void novoServFixo(int Cliente){
        try{
            CidadesRestaurantes c = new CidadesRestaurantes();
            System.out.println("Número de pessoas?");
            int n = buf.nextInt();
            while (n>8){
                System.out.print("\nNão possuimos nenhum veículo com essa capacidade.\nFaça dois serviços para todos serem transportados.\n->");
                n = buf.nextInt();
            }
            System.out.println("Onde se encontra?");
            String s1 = c.EscolhaCidade();
            String veiculoM = atribuicaoVp(c.mainPerto(s1),n);
            System.out.println(veiculoM);
            listaVeic.get(veiculoM).setEstado(false);
            System.out.println("Para onde deseja ir?");
            String s2 = c.EscolhaCidade();
            while (s1==s2){
                System.out.println("Destino invalido. Escolha novamente.\nPara onde deseja ir?");
                s2 = c.EscolhaCidade();
            }
            Double dist = c.CalcDistancia(s1,s2);
            nS++;
            Servico f=new SPessoasF(nS,Cliente,veiculoM,dist,n,s1,s2);
            listaServ.put(nS,f);
            listaVeic.get(veiculoM).setKm(listaVeic.get(veiculoM).getKm()+dist);
            listaVeic.get(veiculoM).setLocalizacao(s2);
            VeicTimer(veiculoM,dist);
        }catch (Exception e){
            System.out.println("Erro do sistema.\nTente novamente.");
        }
    }

    public void novoServCont(int Cliente){
        try {
            CidadesRestaurantes c = new CidadesRestaurantes();
            String[] s = new String[32];
            System.out.println("Número de pessoas?");
            int n = buf.nextInt();
            while (n>8){
                System.out.print("\nNão possuimos nenhum veículo com essa capacidade.\nFaça dois serviços para todos serem transportados.\n->");
                n = buf.nextInt();
            }
            System.out.println("Durante quantos dias prentende manter o serviço?");
            int dias = buf.nextInt();
            while (dias<1){
                System.out.println("O número minimo é um dia.\n->");
                dias = buf.nextInt();
            }
            System.out.println("Onde se encontra?");
            s[0] = c.EscolhaCidade();
            String veiculoM = atribuicaoVp(c.mainPerto(s[0]),n);
            listaVeic.get(veiculoM).setEstado(false);
            System.out.println("Para onde deseja ir?");
            s[1] = c.EscolhaCidade();
            int numCid=1,op=0;
            while (op!=1){
                while (s[numCid-1]==s[numCid]){
                    System.out.println("Destino invalido. Escolha novamente.\nPara onde deseja ir?");
                    s[numCid] = c.EscolhaCidade();
                }
                System.out.println("Deseja terminar a viagem?\n1-Sim\n2-Não");
                op = buf.nextInt();
                if (op==2){
                    numCid++;
                    System.out.println("Para onde deseja ir a seguir?");
                    s[numCid] = c.EscolhaCidade();
                }
            }
            Double dist = c.CalcDistanciaC(numCid,s);
            nS++;
            Servico f=new SPessoasC(nS,Cliente,veiculoM,dist,n,dias,s,numCid);
            listaServ.put(nS,f);
            listaVeic.get(veiculoM).setKm(listaVeic.get(veiculoM).getKm()+dist);
            listaVeic.get(veiculoM).setLocalizacao(s[numCid]);
            VeicTimer(veiculoM,dist/1.5);
        }catch (Exception e){
            System.out.println("Erro do sistema.\nTente novamente.");
        }
    }

    public void novoServMerc(int Cliente){
        try{
            CidadesRestaurantes c = new CidadesRestaurantes();
            System.out.println("Peso da mercadoria a transportar?");
            Double n = buf.nextDouble();
            while (n>15000){
                System.out.print("\nLimite máximo de carga que podemos transportar é 15000kg.\n->");
            }
            System.out.println("Onde se encontra?");
            String s1 = c.EscolhaCidade();
            String veiculoM = atribuicaoVM(c.mainPerto(s1),n);
            listaVeic.get(veiculoM).setEstado(false);
            System.out.println("Para onde deseja ir?");
            String s2 = c.EscolhaCidade();
            while (s1==s2){
                System.out.println("Destino invalido. Escolha novamente.\nPara onde deseja ir?");
                s2 = c.EscolhaCidade();
            }
            Double dist = c.CalcDistancia(s1,s2);
            nS++;
            Servico f=new SMercadorias(nS,Cliente,veiculoM,dist,s1,s2,n);
            listaServ.put(nS,f);
            listaVeic.get(veiculoM).setKm(listaVeic.get(veiculoM).getKm()+dist);
            listaVeic.get(veiculoM).setLocalizacao(s2);
            VeicTimer(veiculoM,dist);
        }catch (Exception e){
            System.out.println("Erro do sistema.\nTente novamente.");
        }
    }

    public void novoServRefeicao(int Cliente){
        try{
            CidadesRestaurantes c = new CidadesRestaurantes();
            System.out.println("Onde desejar comer?");
            String r = c.EscolhaRest();
            Double p = c.precoRefeicacao(r);
            System.out.println("Onde se encontra?");
            String s = c.EscolhaCidade();
            String veiculoM = atribuicaoVR(c.mainPertoR(r));
            listaVeic.get(veiculoM).setEstado(false);
            Double dist = c.CalcDistRest(s,r);
            nS++;
            Servico f=new SRefeicoes(nS,Cliente,veiculoM,dist,r,s,p);
            listaServ.put(nS,f);
            listaVeic.get(veiculoM).setKm(listaVeic.get(veiculoM).getKm()+dist);
            listaVeic.get(veiculoM).setLocalizacao(s);
            VeicTimer(veiculoM,dist);
        }catch (Exception e){
            System.out.println("Erro do sistema.\nTente novamente.");
            e.printStackTrace();
        }
    }

    public int login(){
        System.out.print("Nome: ");
        buf=new Scanner(System.in);
        String nome = buf.nextLine();
        int n=-1;
        if (nome.equals("ADMIN")){
            n=0;
        }else{
            for (int k : listaClie.keySet()){
                if (listaClie.get(k).getNome().equals(nome)){
                    n=k;
                }
            }
        }
        return n;
    }

    public void menuCliente(int n){
        System.out.print("\n\n\n\nBem vindo "+listaClie.get(n).getNome()+"!!\n\n\n" +
                "1-Trajeto fixo\n" +
                "2-Trajeto Continuo\n" +
                "3-Tranporte de mercadorias\n" +
                "4-Entrega de refeições\n" +
                "5-Os seus serviços\n"+
                "Op-> ");
    }

    public void menuAdmin(){
        System.out.print("EMPRESA:\n\n\n" +
                "1-Adicionar veículo\n" +
                "2-Lista de veiculos\n" +
                "3-Lista de veiculos por cliente\n" +
                "4-Lista de serviços\n" +
                "5-Lista de serviços por veiculo\n"+
                "6-Lista de serviços por cliente\n"+
                "7-Serviço mais popular\n"+
                "8-Lista de serviços com distância\n"+
                "9-Total faturado\n"+
                "10-Lista de Clientes\n"+
                "0-Sair\n"+
                "Op-> ");
    }

    public void printListaVei(){
       try{
           System.out.println("Veículos Ligeiros: ");
           for (String k: listaVeic.keySet()){
               if (listaVeic.get(k) instanceof VPessoas){
                   printVeiP(k);
                   System.out.println("\n");
               }
           }
           System.out.println("\nVeículos de mercadorias: ");
           for (String k: listaVeic.keySet()){
               if (listaVeic.get(k) instanceof VMercadorias){
                   printVeiM(k);
                   System.out.println("\n");
               }
           }
           System.out.println("\n\n");
       }catch (Exception e){
           System.out.println("Não existem dados suficientes.");
       }
    }

    public void printListaVeipCli(){
        try{
            HashMap<Integer, ArrayList<Veiculo>> VeiPorCli = new HashMap<>();
            for (int k : listaServ.keySet()){
                int Cliente=listaServ.get(k).getNumeroCliente();
                String Matricula=listaServ.get(k).getMatricula();
                if (VeiPorCli.containsKey(Cliente)){
                    VeiPorCli.get(Cliente).add(listaVeic.get(Matricula).clone());
                }else{
                    ArrayList<Veiculo> v= new ArrayList<>();
                    v.add(listaVeic.get(Matricula).clone());
                    VeiPorCli.put(Cliente,v);
                }
            }
            for (int j: VeiPorCli.keySet()){
                System.out.println("Cliente: "+listaClie.get(j).getNome());
                for (int i=0;i<VeiPorCli.get(j).size();i++){
                    if (VeiPorCli.get(j).get(i) instanceof VPessoas){
                        printVeiP(VeiPorCli.get(j).get(i).getMatricula());
                    }else if (VeiPorCli.get(j).get(i) instanceof VMercadorias){
                        printVeiM(VeiPorCli.get(j).get(i).getMatricula());
                    }
                }
                System.out.println("\n");
            }
        }catch (Exception e){
            System.out.println("Não existem dados suficientes.");
        }
    }

    public void printListaServpVeiculo(){
        try{
            HashMap<String, ArrayList<Servico>> SerPorVeiculo = new HashMap<>();
            for (int k: listaServ.keySet()){
                String V=listaServ.get(k).getMatricula();
                if (SerPorVeiculo.containsKey(V)){
                    SerPorVeiculo.get(V).add(listaServ.get(k).clone());
                }else{
                    ArrayList<Servico> s = new ArrayList<>();
                    s.add(listaServ.get(k).clone());
                    SerPorVeiculo.put(V,s);
                }
            }
            for (String j: SerPorVeiculo.keySet()){
                System.out.println("Matricula do Veículo: "+listaVeic.get(j).getMatricula());
                for (int i=0;i<SerPorVeiculo.get(j).size();i++){
                    if (SerPorVeiculo.get(j).get(i) instanceof SPessoasF){
                        printServF(SerPorVeiculo.get(j).get(i).getNumeroServico());
                    }else if (SerPorVeiculo.get(j).get(i) instanceof SPessoasC){
                        printServC(SerPorVeiculo.get(j).get(i).getNumeroServico());
                    }else if (SerPorVeiculo.get(j).get(i) instanceof SMercadorias){
                        printServM(SerPorVeiculo.get(j).get(i).getNumeroServico());
                    }else if (SerPorVeiculo.get(j).get(i) instanceof SRefeicoes){
                        printServR(SerPorVeiculo.get(j).get(i).getNumeroServico());
                    }
                }
                System.out.println("\n");
            }
        }catch (Exception e){
            System.out.println("Não existem dados suficientes.");
        }
    }

    public void printListaServpCliente(){
        try{
            HashMap<Integer, ArrayList<Servico>> SerPorCliente = new HashMap<>();
            for (int k: listaServ.keySet()){
                int Cliente=listaServ.get(k).getNumeroCliente();
                if (SerPorCliente.containsKey(Cliente)){
                    SerPorCliente.get(Cliente).add(listaServ.get(k).clone());
                }else{
                    ArrayList<Servico> s = new ArrayList<>();
                    s.add(listaServ.get(k).clone());
                    SerPorCliente.put(Cliente,s);
                }
            }
            for (int j: SerPorCliente.keySet()){
                System.out.println("Nome do cliente: "+listaClie.get(j).getNome());
                for (int i=0;i<SerPorCliente.get(j).size();i++){
                    if (SerPorCliente.get(j).get(i) instanceof SPessoasF){
                        printServF(SerPorCliente.get(j).get(i).getNumeroServico());
                    }else if (SerPorCliente.get(j).get(i) instanceof SPessoasC){
                        printServC(SerPorCliente.get(j).get(i).getNumeroServico());
                    }else if (SerPorCliente.get(j).get(i) instanceof SMercadorias){
                        printServM(SerPorCliente.get(j).get(i).getNumeroServico());
                    }else if (SerPorCliente.get(j).get(i) instanceof SRefeicoes){
                        printServR(SerPorCliente.get(j).get(i).getNumeroServico());
                    }
                    System.out.println("\n");
                }
                System.out.println("\n");
            }
        }catch (Exception e){
            System.out.println("Não existem dados suficientes.");
        }
    }

    public void printListaClientes(){
        try {
            for (int k: listaClie.keySet()){
                printCliente(k);
                System.out.println("\n");
            }
        }catch (Exception e){
            System.out.println("Não existem dados suficientes.");
        }
    }

    public void popular(){
        try {
            int[] p = new int[] {0,0,0,0};
            int m=0;
            String s=new String();
            for (int k: listaServ.keySet()){
                if (listaServ.get(k) instanceof SPessoasF){
                    p[0]++;
                }else if (listaServ.get(k) instanceof SPessoasC){
                    p[1]++;
                }else if (listaServ.get(k) instanceof SMercadorias){
                    p[2]++;
                }else if (listaServ.get(k) instanceof SRefeicoes){
                    p[3]++;
                }
            }
            for (int i=0;i<4;i++){
                if (m<p[i]){
                    m=p[i];
                    switch (i){
                        case 0:
                            s="Fixo";
                            break;
                        case 1:
                            s="Continuo";
                            break;
                        case 2:
                            s="Mercadorias";
                            break;
                        case 3:
                            s="Refeições";
                            break;
                    }
                }
            }
            System.out.println("\nServiço mais popular é o "+s+".\n");
        }catch (Exception e){
            System.out.println("Não existem dados suficientes.");
        }
    }

    public void DistServ(){
        try {
            TreeMap<Double,ArrayList<Servico>> a = new TreeMap<>();
            for (int k : listaServ.keySet()){
                if (a.containsKey(listaServ.get(k).getDistanciaPercorrida())){
                    a.get(listaServ.get(k).getDistanciaPercorrida()).add(listaServ.get(k).clone());
                }else{
                    ArrayList<Servico> s = new ArrayList<>();
                    s.add(listaServ.get(k).clone());
                    a.put(listaServ.get(k).getDistanciaPercorrida(),s);
                }
            }
            for (Double k: a.keySet()){
                System.out.println("Distância - "+k+"km:");
                for (int i=0;i<a.get(k).size();i++){
                    System.out.println("\tNº do Serviço: "+a.get(k).get(i).getNumeroServico());
                }
                System.out.println("\n");
            }
        }catch (Exception e){
            System.out.println("Não existem dados suficientes.");
        }
    }

    public void servClient(int k){
        try{
            ArrayList<Servico> s = new ArrayList<Servico>();
            for (int i: listaServ.keySet()){
                if (listaServ.get(i).getNumeroCliente()==k){
                    s.add(listaServ.get(i).clone());
                    if (listaServ.get(i) instanceof SPessoasF){
                        printServF(i);
                    }else if (listaServ.get(i) instanceof SPessoasC){
                        printServC(i);
                    }else if (listaServ.get(i) instanceof SMercadorias){
                        printServM(i);
                    }else if (listaServ.get(i) instanceof SRefeicoes){
                        printServR(i);
                    }
                }
                System.out.println("\n");
            }
        }catch (Exception e){
            System.out.println("Não existem dados suficientes.");
        }
    }

    public void TotalFaturado(){
        try {
            HashMap<LocalDate,Double> c = new HashMap<>();
            for (int k : listaServ.keySet()){
                if (c.containsKey(listaServ.get(k).getData())){
                    Double p=c.get(listaServ.get(k).getData());
                    if (listaServ.get(k) instanceof SPessoasF){
                        p=c.get(listaServ.get(k).getData())+((SPessoasF) listaServ.get(k)).getPrecoF();
                    }else if (listaServ.get(k) instanceof SPessoasC){
                        p=c.get(listaServ.get(k).getData())+((SPessoasC) listaServ.get(k)).getPrecoC();
                    }else if (listaServ.get(k) instanceof SMercadorias){
                        p=c.get(listaServ.get(k).getData())+((SMercadorias) listaServ.get(k)).getPrecoM();
                    }else if (listaServ.get(k) instanceof SRefeicoes) {
                        p=c.get(listaServ.get(k).getData())+((SRefeicoes) listaServ.get(k)).getPrecoTotal();
                    }
                    c.put(listaServ.get(k).getData(),p);
                }else{
                    if (listaServ.get(k) instanceof SPessoasF){
                        c.put(listaServ.get(k).getData(),((SPessoasF) listaServ.get(k)).getPrecoF());
                    }else if (listaServ.get(k) instanceof SPessoasC){
                        c.put(listaServ.get(k).getData(),((SPessoasC) listaServ.get(k)).getPrecoC());
                    }else if (listaServ.get(k) instanceof SMercadorias){
                        c.put(listaServ.get(k).getData(),((SMercadorias) listaServ.get(k)).getPrecoM());
                    }else if (listaServ.get(k) instanceof SRefeicoes) {
                        c.put(listaServ.get(k).getData(),((SRefeicoes) listaServ.get(k)).getPrecoTotal());
                    }
                }
            }
            for (LocalDate d: c.keySet()){
                System.out.println("Data:\t"+d+"\tTotal Faturado:\t"+c.get(d)+"€");
            }
            System.out.println("\n");
        }catch (Exception e){
            System.out.println("Não existem dados suficientes.");
        }
    }

    public void printListaServ(){
        for (int k : listaServ.keySet()){
            if (listaServ.get(k) instanceof SPessoasF){
                printServF(k);
            }
            if (listaServ.get(k) instanceof SMercadorias){
               printServM(k);
            }
            if (listaServ.get(k) instanceof SRefeicoes){
                printServR(k);
            }
            if (listaServ.get(k) instanceof SPessoasC){
                printServC(k);
            }
            System.out.println("\n");
        }
    }

    public boolean disponibilidadeP(){
        for (String k : listaVeic.keySet()){
            if (listaVeic.get(k) instanceof  VPessoas){
                if (listaVeic.get(k).getEstado()){
                    return true;
                }
            }
        }
        return false;
    }

    public boolean disponibilidadeM(){
        for (String k : listaVeic.keySet()){
            if (listaVeic.get(k) instanceof VMercadorias){
                if (listaVeic.get(k).getEstado()){
                    return true;
                }
            }
        }
        return false;
    }

    private void printServF(int k){
        System.out.println("Nº Serviço: " + listaServ.get(k).getNumeroServico()+
                "\n\tData do serviço:          " + listaServ.get(k).getData()+
                "\n\tNº Cliente:               " + listaServ.get(k).getNumeroCliente()+
                "\n\tMatricula do carro do S:  " + listaServ.get(k).getMatricula()+
                "\n\tDistância percorrida:     " + listaServ.get(k).getDistanciaPercorrida()+"km"+
                "\n\tNº Pessoas transportadas: " + ((SPessoasF) listaServ.get(k)).getNumeroPessoas()+
                "\n\tLocal inicial:            " + ((SPessoasF) listaServ.get(k)).getLocalizacao()+
                "\n\tDestino:                  " + ((SPessoasF) listaServ.get(k)).getDestino()+
                "\n\tPreço:                    " + ((SPessoasF) listaServ.get(k)).getPrecoF() +"€");
    }

    private void printServC(int k){
        System.out.println("Nº Serviço: " + listaServ.get(k).getNumeroServico()+
                "\n\tData do serviço:            " + listaServ.get(k).getData()+
                "\n\tDias de aluguer:            " + ((SPessoasC) listaServ.get(k)).getNumeroDiasAluguer()+
                "\n\tNº Cliente:                 " + listaServ.get(k).getNumeroCliente()+
                "\n\tMatricula do carro do S:    " + listaServ.get(k).getMatricula()+
                "\n\tDistância percorrida:       " + listaServ.get(k).getDistanciaPercorrida()+"km"+
                "\n\tNº Pessoas transportadas:   " + ((SPessoasC) listaServ.get(k)).getNumeroPessoas()+
                "\n\tPreço:                      " + ((SPessoasC) listaServ.get(k)).getPrecoC() +"€"+
                "\n\tPercurso:");
        for (int i=0; i<= ((SPessoasC) listaServ.get(k)).getNumLocais();i++){
            System.out.println("\t\t"+(i+1)+"-"+((SPessoasC) listaServ.get(k)).getLocais()[i]);
        }
    }

    private void printServM(int k){
        System.out.println("Nº Serviço: " + listaServ.get(k).getNumeroServico()+
                "\n\tData do serviço:         " + listaServ.get(k).getData()+
                "\n\tNº Cliente:              " + listaServ.get(k).getNumeroCliente()+
                "\n\tMatricula do carro do S: " + listaServ.get(k).getMatricula()+
                "\n\tLocal inicial:           " + ((SMercadorias) listaServ.get(k)).getLocalizacao()+
                "\n\tDestino:                 " + ((SMercadorias) listaServ.get(k)).getDestino()+
                "\n\tDistância percorrida:    " + listaServ.get(k).getDistanciaPercorrida()+"km"+
                "\n\tPeso da mercadoria:      " + ((SMercadorias) listaServ.get(k)).getPeso()+" kg"+
                "\n\tPreço:                   " + ((SMercadorias) listaServ.get(k)).getPrecoM() +"€");
    }

    private void printServR(int k){
        System.out.println("Nº Serviço: " + listaServ.get(k).getNumeroServico()+
                "\n\tData do serviço:         " + listaServ.get(k).getData()+
                "\n\tNº Cliente:              " + listaServ.get(k).getNumeroCliente()+
                "\n\tMatricula do carro do S: " + listaServ.get(k).getMatricula()+
                "\n\tDistância percorrida:    " + listaServ.get(k).getDistanciaPercorrida()+"km"+
                "\n\tPreço da refeição:       " + ((SRefeicoes) listaServ.get(k)).getPrecoRefeicao() +"€"+
                "\n\tLocal inicial:           " + ((SRefeicoes) listaServ.get(k)).getLocalizacao()+
                "\n\tRestaurante:             " + ((SRefeicoes) listaServ.get(k)).getRestaurante()+
                "\n\tPreço total:             " + ((SRefeicoes) listaServ.get(k)).getPrecoTotal() +"€");
    }

    private void printCliente(int k){
        System.out.println("Nº de Cliente:"+listaClie.get(k).getNumeroCliente()+
                "\n\tNome: "+listaClie.get(k).getNome()+
                "\n\tNIF:  "+listaClie.get(k).getNIF());
    }

    private void printVeiP(String k){
        System.out.println("Matricula: "+listaVeic.get(k).getMatricula()+
                "\n\tCapacidade:        "+((VPessoas) listaVeic.get(k)).getCapacidade()+
                "\n\tLocalização:       "+listaVeic.get(k).getLocalizacao()+
                "\n\tEstado:            "+listaVeic.get(k).getEstado()+
                "\n\tKM do carro por S: "+listaVeic.get(k).getKm()+"km");
    }

    private void printVeiM(String k){
        System.out.println("Matricula: "+listaVeic.get(k).getMatricula()+
                "\n\tCapacidade:        "+((VMercadorias) listaVeic.get(k)).getCapacidade()+"kg"+
                "\n\tLocalização:       "+listaVeic.get(k).getLocalizacao()+
                "\n\tEstado:            "+listaVeic.get(k).getEstado()+
                "\n\tKM do carro por S: "+listaVeic.get(k).getKm()+"km");
    }

    private String atribuicaoVp(TreeMap<Double,ArrayList<String>> t,int n){
        String m=new String();
        for (Double k : t.keySet()){
            for (int x=0; x<t.get(k).size();x++){
                for (String i : listaVeic.keySet()){
                    if (listaVeic.get(i) instanceof VPessoas){
                        if (n<((VPessoas) listaVeic.get(i)).getCapacidade()){
                            if (t.get(k).get(x).equals(listaVeic.get(i).getLocalizacao())){
                                m=listaVeic.get(i).getMatricula();
                                return m;
                            }
                        }
                    }
                }
            }
        }
        return m;
    }

    private String atribuicaoVR(TreeMap<Double,ArrayList<String>> t){
        String m=new String();
        for (Double k: t.keySet()){
            for (int x=0; x<t.get(k).size();x++){
                for (String i : listaVeic.keySet()){
                    if (listaVeic.get(i) instanceof VPessoas){
                        if(t.get(k).get(x).equals(listaVeic.get(i).getLocalizacao())){
                            m=listaVeic.get(i).getMatricula();
                            return m;
                        }
                    }
                }
            }
        }
        return m;
    }

    private String atribuicaoVM(TreeMap<Double,ArrayList<String>> t,Double n){
        String m=new String();
        for (Double k : t.keySet()){
            for (int x=0; x<t.get(k).size();x++){
                for (String i : listaVeic.keySet()){
                    if (listaVeic.get(i) instanceof VMercadorias){
                        if (n<((VMercadorias) listaVeic.get(i)).getCapacidade()){
                            if (t.get(k).get(x).equals(listaVeic.get(i).getLocalizacao())){
                                m=listaVeic.get(i).getMatricula();
                                return m;
                            }
                        }
                    }
                }
            }
        }
        return m;
    }

    private void VeicTimer(String m, Double d){
        Timer t=new Timer();
        int miliS=(d.intValue())*1000;
        System.out.println("\nTempo até o veiculo voltar a ficar disponivel "+(miliS/1000)+" segundos.\n");
        t.schedule(new TimerTask() {
            @Override
            public void run() {
                listaVeic.get(m).setEstado(true);
            }
        },miliS);
    }

    public boolean save(){
        try{
            File fS = new File("servicos.txt");
            if (!fS.exists()){
                fS.createNewFile();
            }
            ObjectOutputStream out = new ObjectOutputStream(new FileOutputStream(fS));
            out.writeObject(listaServ);
            out.flush();
            out.close();
        }catch (IOException E){
            System.out.println("Erro no Output do dados de serviços.");
            return false;
        }

        try{
            File fC = new File("clientes.txt");
            if (!fC.exists()){
                fC.createNewFile();
            }
            ObjectOutputStream out = new ObjectOutputStream(new FileOutputStream(fC));
            out.writeObject(listaClie);
            out.flush();
            out.close();
        }catch (IOException E){
            System.out.println("Erro no Output do dados de cliente.");
            return false;
        }

        try{
            File fV = new File("veiculos.txt");
            if (!fV.exists()){
                fV.createNewFile();
            }
            ObjectOutputStream out = new ObjectOutputStream(new FileOutputStream(fV));
            out.writeObject(listaVeic);
            out.flush();
            out.close();
        }catch (IOException E){
            System.out.println("Erro no Output do dados de veiculos.");
            return false;
        }
        return true;
    }

    public void read() {

        try {
            File fS = new File("servicos.txt");
            if (!fS.exists()) {
                return;
            }
            ObjectInputStream in = new ObjectInputStream(new FileInputStream(fS));
            listaServ=(HashMap) in.readObject();
            in.close();

        } catch (ClassNotFoundException C){
            System.out.println("Classe não encontrada.");
            C.printStackTrace();
        } catch (IOException E) {
            System.out.println("Erro no Input do dados de serviços.");
            E.printStackTrace();
        }

        try {
            File fC = new File("clientes.txt");
            if (!fC.exists()) {
                return;
            }
            ObjectInputStream in = new ObjectInputStream(new FileInputStream(fC));
            listaClie=(HashMap) in.readObject();
            in.close();

        } catch (ClassNotFoundException C){
            System.out.println("Classe não encontrada.");
            C.printStackTrace();
        } catch (IOException E) {
            System.out.println("Erro no Input do dados de clientes.");
            E.printStackTrace();
        }

        try {
            File fV = new File("veiculos.txt");
            if (!fV.exists()) {
                return;
            }
            ObjectInputStream in = new ObjectInputStream(new FileInputStream(fV));
            listaVeic=(HashMap) in.readObject();
            in.close();

        } catch (ClassNotFoundException C){
            System.out.println("Classe não encontrada.");
            C.printStackTrace();
        } catch (IOException E) {
            System.out.println("Erro no Input do dados de veículos.");
            E.printStackTrace();

        }
        for (int k:listaServ.keySet()){
            nS=k;
        }
        for (int k:listaClie.keySet()){
            nC=k;
        }
    }
}
