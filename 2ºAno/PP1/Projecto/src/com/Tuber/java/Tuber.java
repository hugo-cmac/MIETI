package com.Tuber.java;

import java.util.Scanner;

public class Tuber {


    public static void main(String[] args) {
        Scanner buf = new Scanner(System.in);
        Empresa E = new Empresa();
        boolean v=true;
        int cliente;
        E.read();
        while (v){
            System.out.print("1-Registo\n2-Login\n0-Sair\nOp-> ");
            buf = new Scanner(System.in);
            int op = buf.nextInt();
            switch (op){
                case 1:
                    E.novoCliente();
                    break;
                case 2:
                    if ((cliente=E.login())==-1){
                        System.out.println("Login inválido.");
                    }else if (cliente==0){
                        while (op!=0){
                            E.menuAdmin();
                            op = buf.nextInt();
                            switch (op){
                                case 1:
                                    E.novoVeiculo();
                                    break;
                                case 2:
                                    E.printListaVei();
                                    break;
                                case 3:
                                    E.printListaVeipCli();
                                    break;
                                case 4:
                                    E.printListaServ();
                                    break;
                                case 5:
                                    E.printListaServpVeiculo();
                                    break;
                                case 6:
                                    E.printListaServpCliente();
                                    break;
                                case 7:
                                    E.popular();
                                    break;
                                case 8:
                                    E.DistServ();
                                    break;
                                case 9:
                                    E.TotalFaturado();
                                    break;
                                case 10:
                                    E.printListaClientes();
                                    break;
                                case 0:
                                    System.out.println("Log out.");
                                    break;
                                default:
                                    System.out.println("Opção inválida.");
                                    break;
                            }
                        }
                    }else{
                        while (op!=0){
                            E.menuCliente(cliente);
                            op = buf.nextInt();
                            switch (op){
                                case 1:
                                    if (E.disponibilidadeP()){
                                        E.novoServFixo(cliente);
                                    }else{
                                        System.out.println("Não temos veículos disponiveis de momento.");
                                    }
                                    break;
                                case 2:
                                    if (E.disponibilidadeP()){
                                        E.novoServCont(cliente);
                                    }else{
                                        System.out.println("Não temos veículos disponiveis de momento.");
                                    }
                                    break;
                                case 3:
                                    if (E.disponibilidadeM()){
                                        E.novoServMerc(cliente);
                                    }else{
                                        System.out.println("Não temos veículos disponiveis de momento.");
                                    }
                                    break;
                                case 4:
                                    if (E.disponibilidadeP()){
                                        E.novoServRefeicao(cliente);
                                    }else{
                                        System.out.println("Não temos veículos disponiveis de momento.");
                                    }
                                    break;
                                case 5:
                                    E.servClient(cliente);
                                    break;
                                case 0:
                                    System.out.println("Log out.");
                                    break;
                                default:
                                    System.out.println("Opção inválida.");
                                    break;
                            }
                        }
                    }
                    op=2;
                    break;
                case 0:
                    if (E.save()){
                        v=false;
                    }else{
                        System.out.println("Erro a gravar no ficheiro, desja sair mesmo assim?\n1-Sim\n2-Não");
                        op=buf.nextInt();
                        if (op==1){
                            v=false;
                        }
                    }
                    System.out.println("A fechar programa.");
                    break;
                default:
                    System.out.println("Opção inválida.");
                    break;
            }
        }
    }
}
