#include "memoria.hpp"

Memoria::Memoria() {
    
}

Memoria::~Memoria() {
    cout << "\n=========================";
    cout << "\n\nMemoria: Liberando recursos...\n";
    // A thread de cada página é automaticamente limpa quando a memória é desalocada
}

void Memoria::inicializarMemoria() {
    
    cout << "Memoria: Inicializando...\n";

    cout << "Memoria: Inicializada.\n";
}

PaginaMemoria::PaginaMemoria() {
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
}