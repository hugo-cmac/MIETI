package com.Tuber.java;

import java.io.Serializable;

public class Cliente implements Serializable {
    private int NumeroCliente;
    private String Nome;
    private int NIF;

    public Cliente(int numeroCliente, String nome, int NIF) {
        this.NumeroCliente = numeroCliente;
        this.Nome = nome;
        this.NIF = NIF;
    }

    public int getNumeroCliente() {
        return NumeroCliente;
    }

    public String getNome() {
        return Nome;
    }

    public int getNIF() {
        return NIF;
    }
    public Cliente clone(){
        Cliente t = new Cliente(this.NumeroCliente,this.Nome,this.NIF);
        return t;
    }
}
