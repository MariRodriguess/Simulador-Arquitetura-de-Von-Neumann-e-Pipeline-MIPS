#ifndef CPU_HPP
#define CPU_HPP

#include "include.hpp"
#include "unidadeControle.hpp"

#define NUM_NUCLEOS 1 // Define o número de núcleos

struct NucleoArgs {
    int *registradores; // Registradores do núcleo
    int idNucleo;       // ID do núcleo
    string nomeArquivo; // Arquivo de instruções
};

class CPU {

    private:
        int *registradores;           // Array de registradores compartilhados
        vector<int *> nucleos;        // Registradores dos núcleos
        vector<bool> nucleosLivres;   // Estado dos núcleos (livres ou ocupados)

    public:
        int id;
        CPU();
        ~CPU();
        pthread_mutex_t mutexCond;     // Mutex para sincronização de threads
        pthread_cond_t cond;           // Variável de condição para sincronização

        void inicializarCpu();
        void exibirEstado();
};


#endif
