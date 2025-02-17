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
#include <algorithm>
#include <chrono>

#define NUM_CPUS 2
#define NUM_PERIFERICOS 5

using namespace std;

extern int PC;
extern int CLOCK[NUM_CPUS];
extern int tempoGasto[NUM_CPUS]; 
extern int contProcessos[NUM_CPUS];
extern volatile bool FCFS;
extern volatile bool RoundRobin;
extern volatile bool Loteria;
extern volatile bool SJF;
extern volatile bool SJF_map;
extern unordered_map<int, int> cache;
extern vector<int> principal;
extern vector<vector<int>> disco;
extern bool perifericos[NUM_PERIFERICOS];
extern int processoFinalizado;
extern pthread_mutex_t mtx;

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
    int numBilhetes = 0;
    bool recebeuRecurso = false;
    int quantumNecessario = 0;
    int quantumOriginal;

    PCB(){
        timestamp = 0;
        linhasProcessadasAnt = 0;
        linhasProcessadasAtual = 0;
        quantum = 0;
        quantumNecessario = 0;
    }
};

struct PaginaVirtual {
    string enderecoVirtual; // ID em binário (endereço virtual)
    int quantumNecessario;  // Quantum necessário para execução
};
extern vector<PaginaVirtual> paginasVirtuais;



#endif
