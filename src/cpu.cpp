#include "cpu.hpp"
#include "functions.hpp"
#include <pthread.h>


CPU::CPU(string nomeArquivo) : nomeArquivo(nomeArquivo), pc(0), clock(0) {
    registradores = (int *)malloc(32 * sizeof(int));
    //pthread_mutex_init(&lock, nullptr); // Inicializa o mutex
}

CPU::~CPU() {
    free(registradores);
    //pthread_mutex_destroy(&lock); // Destroi o mutex
}


void CPU::inicializar() {
    cout << "Inicializando CPU..." << endl;

    for (int i = 0; i < NUM_NUCLEOS; i++) {
        int *reg = (int *)malloc(32 * sizeof(int)); // Registradores por núcleo
        for (int j = 0; j < 32; j++) {
            reg[j] = 0; // Inicializa registradores com zero
        }
        nucleos.push_back(reg);
        nucleosLivres.push_back(true); // Todos os núcleos começam livres
    }

    //pthread_mutex_init(&lockPipeline, nullptr); // Inicializa o mutex do pipeline
}

void CPU::executar() {
    cout << "Iniciando execução das instruções em " << NUM_NUCLEOS << " núcleos..." << endl;

    pthread_t threads[NUM_NUCLEOS];      // Array de threads
    NucleoArgs args[NUM_NUCLEOS];       // Argumentos para os núcleos

    for (int i = 0; i < NUM_NUCLEOS; i++) {
        args[i] = {nucleos[i], i, nomeArquivo}; // Define argumentos
        pthread_create(&threads[i], nullptr, executarNucleo, &args[i]);
    }

    for (int i = 0; i < NUM_NUCLEOS; i++) {
        pthread_join(threads[i], nullptr); // Espera as threads finalizarem
    }

    cout << "Execução finalizada em todos os núcleos." << endl;
}

void *executarNucleo(void *arg) {
    
    // Argumentos recebidos pela thread
    auto *dados = static_cast<NucleoArgs *>(arg);

    int *registradores = dados->registradores;
    int idNucleo = dados->idNucleo;
    string nomeArquivo = dados->nomeArquivo;

    cout << "Núcleo " << idNucleo << " iniciado." << endl;

    // Processa instruções para este núcleo
    LerInstrucoesDoArquivo(nomeArquivo, registradores);

    cout << "Núcleo " << idNucleo << " finalizado." << endl;
    return nullptr;
}

void CPU::exibirEstado() {
    cout << "Estado Final da CPU:" << endl;
    for (int i = 0; i < NUM_NUCLEOS; i++) {
        cout << "Núcleo " << i << ": ";
        for (int j = 0; j < 32; j++) {
            cout << nucleos[i][j] << " ";
        }
        cout << endl;
    }
}
