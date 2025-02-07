#include "cpu.hpp"
#include "functions.hpp"

int PC = 0;
int CLOCK[NUM_CPUS] = {0};
int tempoGasto[NUM_CPUS] = {0}; 
int contProcessos[NUM_CPUS] = {0}; 
bool perifericos[NUM_PERIFERICOS] = {true};
volatile bool FCFS=false;
volatile bool RoundRobin=false;
volatile bool Loteria=false;
volatile bool SJF=false;
volatile bool auxiliar=false;
volatile bool MMU=false;

pthread_mutex_t filaLock = PTHREAD_MUTEX_INITIALIZER;

vector<int> principal;

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
        contProcessos[i] = 0;
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
    Loteria=false;
    SJF=false;
    RoundRobin=false;

    // Inicializa as CPUs
    for (int i = 0; i < NUM_CPUS; ++i) {
        cpus[i] = new CPU();
        cpus[i]->id = i + 1; // Identificação da CPU
        cpus[i]->ocupada = false;
    }

    bootloader(memoria);

    string diretorio = "data"; // Pasta contendo os arquivos .data

    if (!auxiliar){ 
        if(MMU){
            LogSaida("--- POLÍTICA DE ESCALONAMENTO: FCFS --- MMU\n================================================================================================="); 
            cout << "\n\n===== POLITICA DE ESCALONAMENTO: FCFS --- MMU =====\n"; 
        }
        else{
            LogSaida("--- POLÍTICA DE ESCALONAMENTO: FCFS\n================================================================================================="); 
            cout << "\n\n===== POLITICA DE ESCALONAMENTO: FCFS =====\n"; 
        }
    }else{ 
        LogSaida("--- ESCALONAMENTO BASEADO EM SIMILARIDADE\n================================================================================================="); 
        cout << "\n\n===== ESCALONAMENTO BASEADO EM SIMILARIDADE =====\n"; 
    }

    carregarProcessos(diretorio);

    cout << "\n\n==Iniciando execucao...==\n\n\n";
    // Cria threads para cada CPU
    vector<pthread_t> threads_cpus(NUM_CPUS);
    for (int i = 0; i < NUM_CPUS; ++i) {
        pthread_create(&threads_cpus[i], nullptr, executarCpu_FCFS, cpus[i]);
        //sleep(1);
    }

    // Aguarda as threads das CPUs terminarem
    for (auto &th : threads_cpus) {
        pthread_join(th, nullptr);
    }

    cout << "\n\n==Execucao finalizada!==\n";
    cout << "\nAcompanhe a execucao dos processos no arquivo de output: 'log_output.txt' :)\n";

    // Libera memória
    for (auto* cpu : cpus) {
        if (cpu) {
            delete cpu;
            cpu = nullptr;  // Evita double free
        }
    }
    if (memoria) {
        delete memoria;
        memoria = nullptr;
    }
    for(int i = 0; i < NUM_CPUS; i++){
        cout << "CLOCK " << i+1 << " = " << CLOCK[i] << " | ";
        LogSaida("CLOCK " + to_string(i+1) + " = " + to_string(CLOCK[i]) + " | ");
    }
    
    LogSaida("PC = " + to_string(PC));
}

void main_Loteria(){

    FCFS=false;
    Loteria=true;
    SJF=false;
    RoundRobin=false;

    vector<CPU*> cpus(NUM_CPUS);

    Memoria* memoria = new Memoria();

    // Inicializa as CPUs
    for (int i = 0; i < NUM_CPUS; ++i) {
        cpus[i] = new CPU();
        cpus[i]->id = i + 1; // Identificação da CPU
        cpus[i]->ocupada = false;
    }

    bootloader(memoria);

    string diretorio = "data"; // Pasta contendo os arquivos .data

    LogSaida("\n\n--- POLÍTICA DE ESCALONAMENTO: Loteria\n=================================================================================================");
    cout << "\n\n==== POLITICA DE ESCALONAMENTO: Loteria ====\n";

    carregarProcessos(diretorio);

    cout << "\n\n==Iniciando execucao...==\n\n\n";
    // Cria threads para cada CPU
    vector<pthread_t> threads_cpus(NUM_CPUS);
    for (int i = 0; i < NUM_CPUS; ++i) {
        pthread_create(&threads_cpus[i], nullptr, executarCpu_Loteria, cpus[i]);
        //sleep(1);
    }

    // Aguarda as threads das CPUs terminarem
    for (auto &th : threads_cpus) {
        pthread_join(th, nullptr);
    }

    cout << "\n\n==Execucao finalizada!==\n";
    cout << "\nAcompanhe a execucao dos processos no arquivo de output: 'log_output.txt' :)\n";

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

void main_SJF(){
    vector<CPU*> cpus(NUM_CPUS);

    Memoria* memoria = new Memoria();

    FCFS=true;
    SJF=true;
    Loteria=false;
    RoundRobin=false;

    // Inicializa as CPUs
    for (int i = 0; i < NUM_CPUS; ++i) {
        cpus[i] = new CPU();
        cpus[i]->id = i + 1; // Identificação da CPU
        cpus[i]->ocupada = false;
    }

    bootloader(memoria);

    string diretorio = "data"; // Pasta contendo os arquivos .data

    LogSaida("\n\n--- POLÍTICA DE ESCALONAMENTO: SJF\n=================================================================================================");
    cout << "\n\n===== POLITICA DE ESCALONAMENTO: SJF =====\n";

    carregarProcessos(diretorio);

    cout << "\n\n==Iniciando execucao...==\n\n\n";
    // Cria threads para cada CPU
    vector<pthread_t> threads_cpus(NUM_CPUS);
    for (int i = 0; i < NUM_CPUS; ++i) {
        pthread_create(&threads_cpus[i], nullptr, executarCpu_SJF, cpus[i]);
        //sleep(1);
    }

    // Aguarda as threads das CPUs terminarem
    for (auto &th : threads_cpus) {
        pthread_join(th, nullptr);
    }

    cout << "\n\n==Execucao finalizada!==\n";
    cout << "\nAcompanhe a execucao dos processos no arquivo de output: 'log_output.txt' :)\n";

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

void main_RoundRobin() {
    vector<CPU*> cpus(NUM_CPUS);

    Memoria* memoria = new Memoria();

    FCFS=false;
    SJF=false;
    Loteria=false;
    RoundRobin=true;

    // Inicializa as CPUs
    for (int i = 0; i < NUM_CPUS; ++i) {
        cpus[i] = new CPU();
        cpus[i]->id = i + 1; // Identificação da CPU
        cpus[i]->ocupada = false;
    }

    bootloader(memoria);

    string diretorio = "data"; // Pasta contendo os arquivos .data

    LogSaida("\n\n--- POLÍTICA DE ESCALONAMENTO: Round Robin\n=================================================================================================");
    cout << "\n\n===== POLITICA DE ESCALONAMENTO: Round Robin =====\n";

    carregarProcessos(diretorio);

    cout << "\n\n==Iniciando execucao...==\n\n\n";

    // Cria threads para cada CPU
    vector<pthread_t> threads_cpus(NUM_CPUS);
    for (int i = 0; i < NUM_CPUS; ++i) {
        pthread_create(&threads_cpus[i], nullptr, executarCpu_RoundRobin, cpus[i]);
    }

    // Aguarda as threads das CPUs terminarem
    for (auto& th : threads_cpus) {
        pthread_join(th, nullptr);
    }

    cout << "\n\n==Execucao finalizada!==\n";
    cout << "\nAcompanhe a execucao dos processos no arquivo de output: 'log_output.txt' :)\n";

    // Libera memória
    for (auto* cpu : cpus) {
        delete cpu;
    }
    delete memoria;

    for (int i = 0; i < NUM_CPUS; i++) {
        cout << "CLOCK " << i + 1 << " = " << CLOCK[i] << " | ";
        LogSaida("CLOCK " + to_string(i + 1) + " = " + to_string(CLOCK[i]) + " | ");
    }
    cout << "\nPC = " << PC << endl;

    LogSaida("PC = " + to_string(PC));
}

void limparArquivo(){
    const string fileName = "log_output.txt";

    //Limpeza do arquivo de log
    ofstream file(fileName, ios::out | ios::trunc);

    if (file.is_open()) {
        file.close();
    } else {
        cerr << "Erro ao abrir o arquivo '" << fileName << "'.\n";
    }
}

void mostrarMenu() {
    cout << "\n\n=============== Menu de Escalonadores ===============" << endl; 
    cout << "1. Rodar Escalonador FCFS (First Come First Serve)" << endl; 
    cout << "2. Rodar Escalonador SJF (Shortest Job First)" << endl; 
    cout << "3. Rodar Escalonador Loteria" << endl; 
    cout << "4. Rodar Escalonador Round Robin" << endl; 
    cout << "5. Rodar Escalonamento Baseado em Similaridade" << endl; 
    cout << "6. Rodar Todos os Escalonadores" << endl; 
    cout << "7. MMU - FCFS" << endl;
    cout << "0. Sair" << endl; 
    cout << "=====================================================" << endl; 
    cout << "Digite sua escolha: "; 
}

int main() {
    auto inicio = chrono::high_resolution_clock::now();
    auto fim = chrono::high_resolution_clock::now();
    auto duracao = chrono::duration_cast<std::chrono::milliseconds>(fim - inicio);
    limparArquivo();
    limpeza();

    int opcao = -1;

    while (opcao != 0) {
        mostrarMenu();
        cin >> opcao;

        switch (opcao) {
            case 1:
                limparArquivo();
                inicio = chrono::high_resolution_clock::now();
                main_FCFS();
                fim = chrono::high_resolution_clock::now();
                duracao = chrono::duration_cast<std::chrono::milliseconds>(fim - inicio); // Calcula a duração
                cout << "Tempo de execução: " << duracao.count() << " ms" << endl;
                limpeza();
                break;
            case 2:
                limparArquivo();
                main_SJF();
                limpeza();
                break;
            case 3:
                limparArquivo();
                main_Loteria();
                limpeza();
                break;
            case 4:
                limparArquivo();
                main_RoundRobin();
                limpeza();
                break;
            case 5: 
                auxiliar = true; 
                limparArquivo();
                main_FCFS(); 
                limpeza(); 
                auxiliar = false; 
                break; 
            case 6: 
                limparArquivo(); 
 
                main_FCFS(); 
                limpeza(); 
 
                main_Loteria(); 
                limpeza(); 
                 
                main_SJF(); 
                limpeza(); 
 
                main_RoundRobin(); 
                limpeza(); 
                break;
            case 7:
                MMU = true;
                limparArquivo();
                main_FCFS(); 
                limpeza(); 
                MMU = false;
                break;
            case 0:
                cout << "Saindo do programa..." << endl;
                break;
            default:
                cout << "Opcao invalida! Tente novamente." << endl;
        }

        cout << endl;
    }

    return 0;
}