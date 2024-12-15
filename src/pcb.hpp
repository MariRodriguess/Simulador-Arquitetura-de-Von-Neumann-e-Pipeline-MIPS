#ifndef PCB_HPP
#define PCB_HPP

#include "include.hpp"

struct PCB {
    int id; // ID do processo
    string nomeArquivo; // Nome do arquivo associado
    int estado; // Estado: 0 = Pronto, 1 = Executando, 2 = Bloqueado
    int quantum;    // Quantum alocado
    int contadorPrograma;   // Contador de programa do processo
    int registradores[32];  // Estado dos registradores
};

#endif
