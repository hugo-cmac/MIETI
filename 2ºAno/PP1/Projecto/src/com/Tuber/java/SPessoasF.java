package com.Tuber.java;

public class SPessoasF extends Servico {
    private int NumeroPessoas;
    private String Localizacao;
    private String Destino;
    private Double precoF;

    public SPessoasF(int numeroServico, int numeroCliente, String matricula, Double distanciaPercorrida, int numeroPessoas, String localizacao, String destino) {
        super(numeroServico, numeroCliente, matricula, distanciaPercorrida);
        this.NumeroPessoas = numeroPessoas;
        this.Localizacao = localizacao;
        this.Destino = destino;
        this.precoF = distanciaPercorrida*1.5;
    }

    public int getNumeroPessoas() {
        return NumeroPessoas;
    }

    public String getLocalizacao() {
        return Localizacao;
    }

    public String getDestino() {
        return Destino;
    }

    public Double getPrecoF() {
        return precoF;
    }
    public SPessoasF clone(){
        SPessoasF s = new SPessoasF(super.getNumeroServico(),super.getNumeroCliente(),super.getMatricula(),super.getDistanciaPercorrida(),this.NumeroPessoas,this.Localizacao,this.Destino);
        return s;
    }
}
