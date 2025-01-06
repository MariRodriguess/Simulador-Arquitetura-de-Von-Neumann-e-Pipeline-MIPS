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
#include <semaphore.h>

#define NUM_PERIFERICOS 5
#define NUM_CPUS 2

using namespace std;

extern int PC;
extern int CLOCK[NUM_CPUS];
extern int tempoGasto[NUM_CPUS]; 

extern unordered_map<int, int> cache;
extern vector<int> principal;
extern vector<vector<int>> disco;

extern bool perifericos[NUM_PERIFERICOS];

extern int processoFinalizado;

struct PCB {
    int id;               // Identificador do processo/thread
    int prioridade;       // Prioridade do processo
    int quantum;          // Quantum de tempo
    int timestamp; 
    int *registradores;   // Registradores do processo
    string caminhoArquivo; // Caminho do arquivo .data
    int linhasArquivo;      // Número de linhas no arquivo original
    int linhasProcessadasAnt; // Número de linhas que já foram processadas
    int linhasProcessadasAtual;
    string estado;     // Estado atual (e.g., Pronto, Em execução, Bloqueado)
    vector<int> recursos;
    sem_t semaforo; // Semáforo para sincronização
    int idCpuAtual = -1;
    int tempoGasto = 0;
    PCB(){
        linhasProcessadasAnt = 0;
        linhasProcessadasAtual = 0;
        timestamp = 0;
    }
};

#endif
