package com.Tuber.java;

public class SMercadorias extends Servico {
    private String Localizacao;
    private String Destino;
    private Double Peso;
    private Double precoM;

    public SMercadorias(int numeroServico, int numeroCliente, String matricula, Double distanciaPercorrida, String localizacao, String destino, double peso) {
        super(numeroServico, numeroCliente, matricula, distanciaPercorrida);
        this.Localizacao = localizacao;
        this.Destino = destino;
        this.Peso = peso;
        this.precoM = (distanciaPercorrida*(peso/2));
    }

    public String getLocalizacao() {
        return Localizacao;
    }

    public String getDestino() {
        return Destino;
    }

    public Double getPeso() {
        return Peso;
    }

    public Double getPrecoM() {
        return precoM;
    }

    public SMercadorias clone(){
        SMercadorias s = new SMercadorias(super.getNumeroServico(),super.getNumeroCliente(),super.getMatricula(),super.getDistanciaPercorrida(),this.Localizacao,this.Destino,this.Peso);
        return s;
    }
}
