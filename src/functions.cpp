#include "unidadeControle.hpp"
#include "functions.hpp"

void LerInstrucoesDoArquivo(const string &nomeArquivo, int *registradores) {
    ifstream arquivo(nomeArquivo);
    string linha;

    if (!arquivo.is_open()) {
        cout << "Erro ao abrir o arquivo!" << endl;
        return;
    }

    while (getline(arquivo, linha)) {
        UnidadeControle(registradores, linha);
        //cout << "Clock: " << CLOCK << endl;
    }

    arquivo.close();
}

void carregarProcessos(const string &diretorio, queue<PCB> &filaProcessos) {
    namespace fs = filesystem;

    int idProcesso = 0;
    for (const auto &entry : fs::directory_iterator(diretorio)) {
        if (entry.is_regular_file() && entry.path().extension() == ".data") {
            PCB processo;
            processo.id = idProcesso++;
            processo.prioridade = rand() % 10;
            processo.quantum = 10;
            processo.registradores = (int *)malloc(32 * sizeof(int));
            processo.caminhoArquivo = entry.path().string();

            pthread_mutex_lock(&filaLock);
            filaProcessos.push(processo);
            pthread_mutex_unlock(&filaLock);

            cout << "Carregado processo " << processo.id << " do arquivo: " 
                 << processo.caminhoArquivo << endl;
        }
    }
}

void LogSaida(const string &mensagem) {
    ofstream logFile("log_output.txt", ios::app); // Abre o arquivo no modo append
    time_t agora = time(0);                       // Pega o tempo atual
    char *dt = ctime(&agora);                     // Converte para string legível

    // Remove o '\n' ao final da string de tempo
    string tempoAtual(dt);
    tempoAtual.pop_back();

    // Mensagem formatada
    cout << "[" << tempoAtual << "] " << mensagem << endl; // Exibe no console
    if (logFile.is_open()) {
        logFile << "[" << tempoAtual << "] " << mensagem << endl; // Escreve no arquivo
        logFile.close();
    }
}

/*
// bootloader.cpp
void bootloader() {
    std::cout << "Bootloader: Inicialização iniciada...\n";

    // Instâncias das classes
    CPU cpu;
    Memoria memoria;
    SistemaOperacional so;

    // Criação das threads
    pthread_t th_cpu, th_mem, th_so;

    // Passa a instância da CPU para a thread e chama o método de inicialização
    pthread_create(&th_cpu, nullptr, [](void* arg) -> void* {
        CPU* cpuInstance = static_cast<CPU*>(arg); 
        cpuInstance->inicializarCpu();
        return nullptr;
    }, &cpu);

    // Criação de threads para Memoria e SistemaOperacional
    pthread_create(&th_mem, nullptr, &Memoria::inicializar, nullptr);
    pthread_create(&th_so, nullptr, &SistemaOperacional::inicializar, nullptr);

    // Aguarda as threads terminarem
    pthread_join(th_cpu, nullptr);
    pthread_join(th_mem, nullptr);
    pthread_join(th_so, nullptr);

    std::cout << "Bootloader: Arquitetura inicializada com sucesso!\n";
}
*/

