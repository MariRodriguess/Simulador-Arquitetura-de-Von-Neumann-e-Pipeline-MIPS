#include "cpu.hpp"
#include "functions.hpp"

CPU::CPU(string nomeArquivo) : nomeArquivo(nomeArquivo), pc(0), clock(0) {
    registradores = (int *)malloc(32 * sizeof(int));
}

CPU::~CPU() {
    free(registradores);
}

void CPU::inicializar() {

    cout << "Inicializando CPU..." << endl;

    for (int i = 0; i < 8; i++) {
        
        for (int j = 0; j < 32; j++) {
         
            registradores[j] = 0; // Inicializa todos os registradores com zero

        }

        nucleos.push_back(registradores);
    }
}

void CPU::executar() {
    cout << "Iniciando execucao das instrucoes..." << endl;
    LerInstrucoesDoArquivo(nomeArquivo, registradores);
}

void CPU::exibirEstado() {
    cout << "Estado Final da CPU: ";
    cout << "PC = " << PC << " CLOCK = " << CLOCK << endl;
    cout << "Registradores: ";
    for (int i = 0; i < 32; i++) {
        cout << registradores[i] << " ";
    }
    cout << endl;
}
