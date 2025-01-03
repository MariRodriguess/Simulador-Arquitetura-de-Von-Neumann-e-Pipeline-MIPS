#include "cpu.hpp"
#include "functions.hpp"

int PC = 0;
int CLOCK = 0;
bool perifericos[NUM_PERIFERICOS] = {false};
vector<int> principal;
pthread_mutex_t filaLock = PTHREAD_MUTEX_INITIALIZER;

void bootloader(CPU *cpu, Memoria * memoria) {
    
    cout << "Bootloader: Inicializacao iniciada...\n";

    // Criação das threads
    pthread_t th_cpu, th_mem;

    // Passa a instância da CPU para a thread e chama o método de inicialização
    pthread_create(&th_cpu, nullptr, [](void* arg) -> void* {
        CPU* cpuInstance = static_cast<CPU*>(arg);
        cpuInstance->inicializarCpu();
        return nullptr;
    }, &cpu);

    // Criação de threads para Memoria 
    pthread_create(&th_mem, nullptr,[](void* arg) -> void* {
        Memoria* memInstance = static_cast<Memoria*>(arg);
        memInstance->inicializarMemoria();
        return nullptr;
    }, &memoria);

    cout << "Bootloader: Arquitetura inicializada com sucesso!\n";

    // Aguarda as threads terminarem
    pthread_join(th_cpu, nullptr);
    pthread_join(th_mem, nullptr);
}

int main() {

    CPU *cpu = new CPU();
    Memoria *memoria = new Memoria();

    bootloader(cpu, memoria);

    string diretorio = "data"; // Pasta contendo os arquivos .data

    carregarProcessos(diretorio);
    sleep(5);
    executarCpu();  

    cpu->~CPU();
    memoria->~Memoria();

    cout << "PC = " << PC << " | CLOCK = " << CLOCK << endl;

    return 0;
}