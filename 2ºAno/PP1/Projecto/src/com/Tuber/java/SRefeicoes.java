package com.Tuber.java;

public class SRefeicoes extends Servico {
    private String Restaurante;
    private String Localizacao;
    private Double PrecoRefeicao;
    private Double PrecoTotal;

    public SRefeicoes(int numeroServico, int numeroCliente, String matricula, Double distanciaPercorrida, String restaurante, String localizacao, Double precoRefeicao) {
        super(numeroServico, numeroCliente, matricula, distanciaPercorrida);
        this.Restaurante = restaurante;
        this.Localizacao = localizacao;
        this.PrecoRefeicao = precoRefeicao;
        this.PrecoTotal = (super.getDistanciaPercorrida()+this.PrecoRefeicao);
    }

    public String getRestaurante() {
        return Restaurante;
    }

    public String getLocalizacao() {
        return Localizacao;
    }

    public Double getPrecoRefeicao() {
        return PrecoRefeicao;
    }

    public Double getPrecoTotal() {
        return PrecoTotal;
    }

    public SRefeicoes clone(){
        SRefeicoes s = new SRefeicoes(super.getNumeroServico(),super.getNumeroCliente(),super.getMatricula(),super.getDistanciaPercorrida(),this.Restaurante,this.Localizacao,this.PrecoRefeicao);
        return s;
    }
}
