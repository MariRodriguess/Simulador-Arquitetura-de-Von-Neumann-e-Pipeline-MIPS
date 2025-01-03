#ifndef MEMORIA_HPP
#define MEMORIA_HPP

#include "include.hpp"
#include <iostream>
#include <pthread.h>
#include <vector>
#include <functional>

class Memoria {

    public:
        Memoria();   // Construtor
        ~Memoria();  // Destruidor

        void inicializarMemoria();
        pthread_mutex_t mutexCond;     // Mutex para sincronização de threads
        pthread_cond_t cond;
};

struct PaginaMemoria {
    PCB pcb;               // PCB associado
    pthread_t th_processo;  // Thread vinculada ao processo
    pthread_mutex_t mutex;     // Mutex para sincronização de threads
    pthread_cond_t cond;           // Variável de condição para sincronização

    PaginaMemoria();
};

#endif



