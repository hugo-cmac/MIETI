package com.Tuber.java;

import java.util.*;

public class CidadesRestaurantes {

    private HashMap<String,int[]> Cidade = new HashMap<>();
    private HashMap<String,int[]> Restaurante = new HashMap<>();
    private HashMap<String,Double> RestaurantePrec = new HashMap<>();

    private int[] coords;
    private Scanner buf = new Scanner(System.in);

    public CidadesRestaurantes() {
        coords=new int[] {0,0};
        Cidade.put("Cede - Tuber",coords);
        coords=new int[] {2,-1};
        Cidade.put("Solteira - Tavira",coords);
        coords=new int[] {-3,2};
        Cidade.put("Celorico de Bastos",coords);
        coords=new int[] {4,-3};
        Cidade.put("São Joanico - Vimioso",coords);
        coords=new int[] {-5,4};
        Cidade.put("Pé de Cão - Santarém",coords);
        coords=new int[] {-2,1};
        Cidade.put("Chiqueiro - Lousã",coords);
        coords=new int[] {3,-2};
        Cidade.put("Colo do Pito - Viseu",coords);
        coords=new int[] {-4,3};
        Cidade.put("Quinta de Comichão - Guarda",coords);
        coords=new int[] {5,-4};
        Cidade.put("Venda das Raparigas - Leiria",coords);
        coords=new int[] {6,-5};
        Cidade.put("Cabeçudos - Famalicão",coords);

        coords=new int[] {10,10};
        Restaurante.put("Churrascaria Quim dos Joelhos-Vila Real",coords);
        coords=new int[] {2,2};
        Restaurante.put("McDonalds",coords);
        Restaurante.put("Burguer King",coords);
        Restaurante.put("KFC",coords);
        Restaurante.put("Pizza Hut",coords);
        Restaurante.put("Dominos",coords);
        Restaurante.put("Dunkin Donut",coords);
        Restaurante.put("Wok-to-Walk",coords);
        Restaurante.put("Chibarias",coords);
        RestaurantePrec.put("Churrascaria Quim dos Joelhos-Vila Real",12.5);
        RestaurantePrec.put("McDonalds",6.0);
        RestaurantePrec.put("Burguer King",5.5);
        RestaurantePrec.put("KFC",8.5);
        RestaurantePrec.put("Pizza Hut",18.0);
        RestaurantePrec.put("Dominos",13.5);
        RestaurantePrec.put("Dunkin Donut",2.5);
        RestaurantePrec.put("Wok-to-Walk",4.5);
        RestaurantePrec.put("Chibarias",3.5);
    }

    public String EscolhaCidade(){
        int i=0,op=0;
        String local=new String();
        for (String key : Cidade.keySet()){
            i++;
            System.out.println(i+" - "+key);
        }
        op = buf.nextInt();
        while (op>i || op<=0){
            System.out.println("Opção inválida\nEscolha novamente\n");
            op = buf.nextInt();
        }
        i=0;
        for (String key : Cidade.keySet()){
            i++;
            if (i==op){
                local=key;
            }
        }
        return local;
    }

    public String EscolhaRest(){
        int i=0,op=0;
        String local=new String();
        for (String key : Restaurante.keySet()){
            i++;
            System.out.println(i+" - "+key);
        }
        op = buf.nextInt();
        while (op>i || op<=0){
            System.out.println("Opção inválida\nEscolha novamente\n");
            op = buf.nextInt();
        }
        i=0;
        for (String key : Restaurante.keySet()){
            i++;
            if (i==op){
                local=key;
            }
        }
        return local;
    }

    public Double CalcDistRest(String um,String dois){
        int [] c1=Cidade.get(um);
        int [] c2=Restaurante.get(dois);
        return Math.sqrt((c1[0]-c1[1])*(c1[0]-c1[1]) + (c2[0]-c2[1])*(c2[0]-c2[1]));
    }

    public Double precoRefeicacao(String s){
        Double p = RestaurantePrec.get(s);
        System.out.println("O preço da reifeição é " + p + "€");
        return p;
    }

    public Double CalcDistancia(String um,String dois){
        int [] c1=Cidade.get(um);
        int [] c2=Cidade.get(dois);
        return Math.sqrt((c1[0]-c1[1])*(c1[0]-c1[1]) + (c2[0]-c2[1])*(c2[0]-c2[1]));
    }

    public Double CalcDistanciaC(int tamanho,String[] s){
        Double dist=0.0;
        for (int i=1;i<tamanho;i++){
            int [] c1=Cidade.get(s[i-1]);
            int [] c2=Cidade.get(s[i]);
            dist = dist + Math.sqrt((c1[0]-c1[1])*(c1[0]-c1[1]) + (c2[0]-c2[1])*(c2[0]-c2[1]));
        }
        return dist;
    }

    public TreeMap<Double, ArrayList<String>> mainPerto(String s){
        TreeMap<Double, ArrayList<String>> dist = new TreeMap<>();
        int [] c1=Cidade.get(s);
        for (String k : Cidade.keySet()){
            int [] c2=Cidade.get(k);
            Double d = Math.sqrt((c1[0]-c1[1])*(c1[0]-c1[1]) + (c2[0]-c2[1])*(c2[0]-c2[1]));
            if (dist.containsKey(d)){
                dist.get(d).add(k);
            }else {
                ArrayList<String> c=new ArrayList<>();
                c.add(k);
                dist.put(d,c);
            }
        }
        return dist;
    }

    public TreeMap<Double, ArrayList<String>> mainPertoR(String s){
        TreeMap<Double, ArrayList<String>> dist = new TreeMap<>();
        int [] c1=Restaurante.get(s);
        for (String k : Cidade.keySet()){
            int [] c2=Cidade.get(k);
            Double d = Math.sqrt((c1[0]-c1[1])*(c1[0]-c1[1]) + (c2[0]-c2[1])*(c2[0]-c2[1]));
            if (dist.containsKey(d)){
                dist.get(d).add(k);
            }else {
                ArrayList<String> c=new ArrayList<>();
                c.add(k);
                dist.put(d,c);
            }
        }
        return dist;
    }
}
