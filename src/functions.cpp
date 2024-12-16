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
