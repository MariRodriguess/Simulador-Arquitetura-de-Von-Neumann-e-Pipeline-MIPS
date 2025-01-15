#include "cpu.hpp"

pthread_mutex_t lockPipeline = PTHREAD_MUTEX_INITIALIZER; // Inicializando o mutex

CPU::CPU() {

    ocupada = false;

    for (int i = 0; i < NUM_NUCLEOS; i++) {
        int *reg = (int *)malloc(10 * sizeof(int)); // Registradores por núcleo
        for (int j = 0; j < 10; j++) {
            reg[j] = 0; // Inicializa registradores com zero
        }
        nucleos.push_back(reg);
        //nucleosLivres.push_back(true); // Todos os núcleos começam livres
    }

}

CPU::~CPU() {
    // Libera os recursos de memória
    for (auto& nucleo : nucleos) {
        free(nucleo);
    }

    nucleosLivres.clear();
    nucleosLivres.shrink_to_fit(); 
    nucleos.clear();
}

void CPU::inicializarCpu() {
    
    cout << "CPU: Inicializando...\n";

    cout << "CPU: Inicializada.\n";
}

void CPU::exibirEstado() {
    cout << "Estado Final da CPU:" << endl;
    for (int i = 0; i < NUM_NUCLEOS; i++) {
        cout << "Nucleo " << i << ": ";
        for (int j = 0; j < 10; j++) {
            cout << nucleos[i][j] << " ";
        }
        cout << endl;
    }
}