package com.Tuber.java;

import java.io.Serializable;

public abstract class Veiculo implements Serializable {
    private String Matricula;
    private String Localizacao;
    private Boolean Estado;
    private Double Km;

    public Veiculo(String matricula, String localizacao, Boolean estado, Double km) {
        this.Matricula = matricula;
        this.Localizacao = localizacao;
        this.Estado = estado;
        this.Km = km;
    }

    public String getMatricula() {
        return Matricula;
    }

    public String getLocalizacao() {
        return Localizacao;
    }

    public Boolean getEstado() {
        return Estado;
    }

    public Double getKm() {
        return Km;
    }

    public void setEstado(Boolean estado) {
        Estado = estado;
    }

    public void setLocalizacao(String localizacao) {
        Localizacao = localizacao;
    }

    public void setKm(Double km) {
        Km = km;
    }

    public abstract Veiculo clone();
}
