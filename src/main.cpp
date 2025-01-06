#include "cpu.hpp"
#include "functions.hpp"


int PC = 0;
int CLOCK[NUM_CPUS] = {0};
int tempoGasto[NUM_CPUS] = {0}; 
bool perifericos[NUM_PERIFERICOS] = {true};
vector<int> principal;
pthread_mutex_t filaLock = PTHREAD_MUTEX_INITIALIZER;

void bootloader(/*vector<CPU*> &cpus,*/ Memoria *memoria) {
    cout << "\nBootloader: Inicializacao iniciada...\n";

    // Criação das threads para as CPUs
    /*vector<pthread_t> threads_cpus(cpus.size());
    for (size_t i = 0; i < cpus.size(); ++i) {
        pthread_create(&threads_cpus[i], nullptr, [](void* arg) -> void* {
            CPU* cpuInstance = static_cast<CPU*>(arg);
            cpuInstance->inicializarCpu();
            return nullptr;
        }, cpus[i]);
    }*/

    // Criação da thread para a Memória
    pthread_t th_mem;
    pthread_create(&th_mem, nullptr, [](void* arg) -> void* {
        Memoria* memInstance = static_cast<Memoria*>(arg);
        memInstance->inicializarMemoria();
        return nullptr;
    }, memoria);

    cout << "Bootloader: Arquitetura inicializada com sucesso!\n";

    // Aguarda as threads terminarem
    /*for (auto &th : threads_cpus) {
        pthread_join(th, nullptr);
    }*/
   pthread_join(th_mem, nullptr);
}


int main() {
    
    vector<CPU*> cpus(NUM_CPUS);

    // Inicializa as CPUs
    for (int i = 0; i < NUM_CPUS; ++i) {
        cpus[i] = new CPU();
        cpus[i]->id = i + 1; // Identificação da CPU
    }

    Memoria* memoria = new Memoria();

    bootloader(/*cpus,*/ memoria);

    string diretorio = "data"; // Pasta contendo os arquivos .data

    carregarProcessos(diretorio);

    // Cria threads para cada CPU
    vector<pthread_t> threads_cpus(NUM_CPUS);
    for (int i = 0; i < NUM_CPUS; ++i) {
        pthread_create(&threads_cpus[i], nullptr, executarCpu, cpus[i]);
        sleep(2);
    }

    // Aguarda as threads das CPUs terminarem
    for (auto &th : threads_cpus) {
        pthread_join(th, nullptr);
    }

    // Libera memória
    for (auto* cpu : cpus) {
        delete cpu;
    }
    delete memoria;

    for(int i = 0; i < NUM_CPUS; i++){
        cout << "CLOCK " << i+1 << " = " << CLOCK[i] << " | ";
    }
    cout << "\nPC = " << PC << endl;
    return 0;
}

