#ifndef INCLUDE_HPP
#define INCLUDE_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <queue>
#include <pthread.h> // Inclusão para suporte a threads
#include <mutex>

#define NUM_PERIFERICOS 5

using namespace std;

extern int PC;
extern int CLOCK;

extern unordered_map<int, int> cache;
extern vector<int> principal;
extern vector<vector<int>> disco;

extern bool perifericos[NUM_PERIFERICOS];

struct Processo {
    int quantum;
    int timestamp;
    int id; // Identificador do processo
};

struct PCB {
    int id;               // Identificador do processo
    int prioridade;       // Prioridade do processo
    int quantum;          // Quantum de tempo
    int *registradores;   // Registradores do processo
    string caminhoArquivo; // Caminho do arquivo .data
};

extern queue<PCB> filaProcessos; // Fila de processos
extern pthread_mutex_t filaLock; // Mutex para proteger o acesso à fila


#endif
