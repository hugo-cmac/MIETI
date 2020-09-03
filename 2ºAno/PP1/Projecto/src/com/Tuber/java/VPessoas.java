package com.Tuber.java;

public class VPessoas extends Veiculo {

    private int Capacidade;

    public VPessoas(String matricula, String localizacao, Boolean estado, Double km, int capacidade) {
        super(matricula, localizacao, estado, km);
        this.Capacidade=capacidade;
    }

    public int getCapacidade() {
        return Capacidade;
    }

    public VPessoas clone(){
        VPessoas v = new VPessoas (super.getMatricula(),super.getLocalizacao(),super.getEstado(),super.getKm(),this.Capacidade);
        return v;
    }
}
