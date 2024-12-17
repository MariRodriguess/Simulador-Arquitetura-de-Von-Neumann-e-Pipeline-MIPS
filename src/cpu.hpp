#ifndef CPU_HPP
#define CPU_HPP

#include "include.hpp"
#include "unidadeControle.hpp"

#define NUM_NUCLEOS 2 // Define o número de núcleos

struct NucleoArgs {
    int *registradores; // Registradores do núcleo
    int idNucleo;       // ID do núcleo
    string nomeArquivo; // Arquivo de instruções
};

class CPU {
    private:
        int *registradores;           // Array de registradores compartilhados
        string nomeArquivo;           // Nome do arquivo de entrada
        int pc;                       // Contador de programa compartilhado
        int clock;                    // Contador de clock
        //pthread_mutex_t lock;         // Mutex para sincronização
        vector<int *> nucleos;        // Registradores dos núcleos
        vector<bool> nucleosLivres;   // Estado dos núcleos (livres ou ocupados)
        
    public:
        CPU();
        ~CPU();

        void executar();
        void exibirEstado();
};

void *executarNucleo(void *arg);

#endif
