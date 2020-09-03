package com.Tuber.java;

public class VMercadorias extends Veiculo{
    private int Capacidade;

    public VMercadorias(String matricula, String localizacao, Boolean estado, Double km, int capacidade) {
        super(matricula, localizacao, estado, km);
        this.Capacidade = capacidade;
    }

    public int getCapacidade() {
        return Capacidade;
    }

    public VMercadorias clone(){
        VMercadorias v = new VMercadorias(super.getMatricula(),super.getLocalizacao(),super.getEstado(),super.getKm(),this.Capacidade);
        return v;
    }
}
