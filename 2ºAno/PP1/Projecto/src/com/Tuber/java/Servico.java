package com.Tuber.java;
import java.io.Serializable;
import java.time.LocalDate;

public abstract class Servico implements Serializable {
    private int NumeroServico;
    private int NumeroCliente;
    private String Matricula;
    private Double DistanciaPercorrida;
    private LocalDate data;

    public Servico(int numeroServico, int numeroCliente, String matricula, Double distanciaPercorrida) {
        this.NumeroServico = numeroServico;
        this.NumeroCliente = numeroCliente;
        this.Matricula = matricula;
        this.DistanciaPercorrida = distanciaPercorrida;
        this.data=LocalDate.now();
    }

    public int getNumeroServico() {
        return NumeroServico;
    }

    public int getNumeroCliente() {
        return NumeroCliente;
    }

    public String getMatricula() {
        return Matricula;
    }

    public Double getDistanciaPercorrida() {
        return DistanciaPercorrida;
    }

    public LocalDate getData() {
        return data;
    }

    public abstract Servico clone();

}
