package com.Tuber.java;

public class SPessoasC extends Servico {
    private int NumeroPessoas;
    private int NumeroDiasAluguer;
    private String[] Locais = new String[128];
    private int NumLocais;
    private Double precoC;

    public SPessoasC(int numeroServico, int numeroCliente, String matricula, Double distanciaPercorrida, int numeroPessoas, int numeroDiasAluguer, String[] locais, int numLocais) {
        super(numeroServico, numeroCliente, matricula, distanciaPercorrida);
        this.NumeroPessoas = numeroPessoas;
        this.NumeroDiasAluguer = numeroDiasAluguer;
        this.Locais = locais;
        this.NumLocais = numLocais;
        this.precoC = distanciaPercorrida*1.5;
    }

    public int getNumeroPessoas() {
        return NumeroPessoas;
    }

    public int getNumeroDiasAluguer() {
        return NumeroDiasAluguer;
    }

    public String[] getLocais() {
        return Locais;
    }

    public int getNumLocais() {
        return NumLocais;
    }

    public Double getPrecoC() {
        return precoC;
    }
    public SPessoasC clone(){
        SPessoasC s = new SPessoasC(super.getNumeroServico(),super.getNumeroCliente(),super.getMatricula(),super.getDistanciaPercorrida(),this.NumeroPessoas,this.getNumeroDiasAluguer(),this.Locais,this.NumLocais);
        return s;
    }
}
