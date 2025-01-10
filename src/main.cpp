#include "cpu.hpp"
#include "functions.hpp"

int PC = 0;
int CLOCK[NUM_CPUS] = {0};
int tempoGasto[NUM_CPUS] = {0}; 
bool perifericos[NUM_PERIFERICOS] = {true};
volatile bool FCFS=true;
vector<int> principal;
pthread_mutex_t filaLock = PTHREAD_MUTEX_INITIALIZER;

void bootloader(Memoria *memoria) {
    cout << "\nBootloader: Inicializacao iniciada...\n";

    // Criação da thread para a Memória
    pthread_t th_mem;
    pthread_create(&th_mem, nullptr, [](void* arg) -> void* {
        Memoria* memInstance = static_cast<Memoria*>(arg);
        memInstance->inicializarMemoria();
        return nullptr;
    }, memoria);

    cout << "Bootloader: Arquitetura inicializada com sucesso!\n";

   pthread_join(th_mem, nullptr);
}

void limpeza(){
    PC = 0;
    for(int i = 0; i < NUM_CPUS; i++){
        CLOCK[i] = 0;
        tempoGasto[i] = 0;
    }   

    // Colocando os periféricos como true
    for (int i = 0; i < NUM_PERIFERICOS; ++i) {
        perifericos[i] = true;
    }

}

void main_FCFS(){
    
    vector<CPU*> cpus(NUM_CPUS);

    Memoria* memoria = new Memoria();

    FCFS=true;

    // Inicializa as CPUs
    for (int i = 0; i < NUM_CPUS; ++i) {
        cpus[i] = new CPU();
        cpus[i]->id = i + 1; // Identificação da CPU
    }

    bootloader(memoria);

    string diretorio = "data"; // Pasta contendo os arquivos .data

    LogSaida("--- POLÍTICA DE ESCALONAMENTO: FCFS\n=================================================================================================");

    carregarProcessos(diretorio);

    cout << "\nIniciando execucao...\n";
    // Cria threads para cada CPU
    vector<pthread_t> threads_cpus(NUM_CPUS);
    for (int i = 0; i < NUM_CPUS; ++i) {
        pthread_create(&threads_cpus[i], nullptr, executarCpu_FCFS, cpus[i]);
        sleep(2);
    }

    // Aguarda as threads das CPUs terminarem
    for (auto &th : threads_cpus) {
        pthread_join(th, nullptr);
    }

    cout << "\nExecucao finalizada!\n";

    // Libera memória
    for (auto* cpu : cpus) {
        delete cpu;
    }
    delete memoria;

    for(int i = 0; i < NUM_CPUS; i++){
        cout << "CLOCK " << i+1 << " = " << CLOCK[i] << " | ";
        LogSaida("CLOCK " + to_string(i+1) + " = " + to_string(CLOCK[i]) + " | ");
    }
    cout << "\nPC = " << PC << endl;

    LogSaida("PC = " + to_string(PC));
}

void main_Loteria(){

    FCFS=false;

    vector<CPU*> cpus(NUM_CPUS);

    Memoria* memoria = new Memoria();

    // Inicializa as CPUs
    for (int i = 0; i < NUM_CPUS; ++i) {
        cpus[i] = new CPU();
        cpus[i]->id = i + 1; // Identificação da CPU
    }

    bootloader(memoria);

    string diretorio = "data"; // Pasta contendo os arquivos .data

    LogSaida("\n\n--- POLÍTICA DE ESCALONAMENTO: Loteria com Prioridade\n=================================================================================================");

    carregarProcessos(diretorio);

    cout << "\nIniciando execucao...\n";
    // Cria threads para cada CPU
    vector<pthread_t> threads_cpus(NUM_CPUS);
    for (int i = 0; i < NUM_CPUS; ++i) {
        pthread_create(&threads_cpus[i], nullptr, executarCpu_Loteria, cpus[i]);
        sleep(2);
    }

    // Aguarda as threads das CPUs terminarem
    for (auto &th : threads_cpus) {
        pthread_join(th, nullptr);
    }

    cout << "\nExecucao finalizada!\n";

    // Libera memória
    for (auto* cpu : cpus) {
        delete cpu;
    }
    delete memoria;

    for(int i = 0; i < NUM_CPUS; i++){
        cout << "CLOCK " << i+1 << " = " << CLOCK[i] << " | ";
        LogSaida("CLOCK " + to_string(i+1) + " = " + to_string(CLOCK[i]) + " | ");
    }
    cout << "\nPC = " << PC << endl;

    LogSaida("PC = " + to_string(PC));
}

int main() {

    const string fileName = "log_output.txt";

    //Limpeza do arquivo de log
    ofstream file(fileName, ios::out | ios::trunc);

    if (file.is_open()) {
        file.close();
    } else {
        cerr << "Erro ao abrir o arquivo '" << fileName << "'.\n";
    }
    
    main_FCFS();
    limpeza();

    main_Loteria();
    limpeza();
    
    return 0;
}

