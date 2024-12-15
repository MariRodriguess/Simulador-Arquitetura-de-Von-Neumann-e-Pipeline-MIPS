#ifndef CPU_HPP
#define CPU_HPP

#include "include.hpp"
#include "unidadeControle.hpp"

class CPU {
private:
    int *registradores; // Array de registradores
    string nomeArquivo; // Nome do arquivo de entrada
    int pc;             // Contador de programa
    int clock;          // Contador de clock

    vector<int*> nucleos;

public:
    CPU(string nomeArquivo);
    ~CPU();

    void inicializar();
    void executar();
    void exibirEstado();
};

#endif
